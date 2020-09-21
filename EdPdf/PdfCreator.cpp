#include "PdfCreator.h"

#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

#include <filesystem>
#include <iostream>
#include <algorithm>
#include <Windows.h>

using namespace Yoga2;
using namespace experimental;

PdfCreator::PdfCreator()
{
	m_pCtx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
	m_pDoc = pdf_create_document(m_pCtx);
	pdf_parse_write_options(m_pCtx, &m_Opts, "compress");
}

PdfCreator::~PdfCreator()
{
	if (m_pCtx != nullptr)
	{
		if (m_pDoc != nullptr)
			pdf_drop_document(m_pCtx, m_pDoc);

		fz_flush_warnings(m_pCtx);
		fz_drop_context(m_pCtx);
	}

	ClearPngList();
}

void PdfCreator::AddImageToPdf(const wstring& pngPath)
{
	ClearPngList();
	LoadPngList(pngPath);

	for (auto info : m_vecPng)
		CreatePage(info->name, info->path);
}

void PdfCreator::SavePdf(const wstring& pdfPath)
{
	string sPath = WideStr2UTF8Str(pdfPath);
	pdf_save_document(m_pCtx, m_pDoc, sPath.c_str(), &m_Opts);
}

void PdfCreator::CreatePage(const string& name, const string& path)
{
	int rotate = 0;

	pdf_obj* resources = pdf_new_dict(m_pCtx, m_pDoc, 2);
	fz_buffer* contents = fz_new_buffer(m_pCtx, 1024);

	fz_image* image = fz_new_image_from_file(m_pCtx, path.c_str());
	if (image == nullptr)
	{
		return;
	}
	fz_rect imageRect = { 0, 0, image->w, image->h};

	pdf_obj* subres = pdf_dict_get(m_pCtx, resources, PDF_NAME(XObject));
	if (!subres)
	{
		subres = pdf_new_dict(m_pCtx, m_pDoc, 10);
		pdf_dict_put_drop(m_pCtx, resources, PDF_NAME(XObject), subres);
	}

	pdf_obj* ref = pdf_add_image(m_pCtx, m_pDoc, image);
	pdf_dict_puts(m_pCtx, subres, name.c_str(), ref);
	pdf_drop_obj(m_pCtx, ref);

	fz_drop_image(m_pCtx, image);

	char str[257] = { 0 };
	sprintf_s(str, 256, "%% Draw an image.\nq\n%f 0 0 %f 0 0 cm\n/%s Do\nQ\n",
		imageRect.x1, imageRect.y1, name.c_str());
	fz_append_string(m_pCtx, contents, str);

	pdf_obj* page = pdf_add_page(m_pCtx, m_pDoc, imageRect, rotate, resources, contents);
	pdf_insert_page(m_pCtx, m_pDoc, -1, page);
	pdf_drop_obj(m_pCtx, page);

	fz_drop_buffer(m_pCtx, contents);
	pdf_drop_obj(m_pCtx, resources);
}

string PdfCreator::WideStr2UTF8Str(const std::wstring& wideStr)
{
	if (!wideStr.empty())
	{
		int strSize = (int)wideStr.size();
		int bufSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), strSize, NULL, 0, NULL, NULL);

		if (bufSize > 0)
		{
			std::string utf8Str(bufSize, 0);
			if (0 < WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), strSize, &utf8Str[0], bufSize, NULL, NULL))
				return utf8Str;
		}
	}

	return std::string();
}

void PdfCreator::LoadPngList(const wstring& path)
{
	filesystem::path pngPath(path);
	filesystem::directory_iterator endIter;

	for (filesystem::directory_iterator iter(pngPath); iter != endIter; iter++)
	{
		wstring wsPath = iter->path().wstring();
		if (wsPath.find(L".png") != string::npos || wsPath.find(L".jpg") != string::npos
			|| wsPath.find(L".jpeg") != string::npos)
		{
			string sPath = WideStr2UTF8Str(wsPath);
			size_t pos = sPath.find_last_of("\\");
			long long lastTime = GetLastWriteTime(wsPath);
			m_vecPng.push_back(new FileInfo(sPath.substr(pos + 1), sPath, lastTime));
		}
	}

	std::sort(m_vecPng.begin(), m_vecPng.end(), [](const FileInfo* a, const FileInfo* b) {
		return a->last_write_time < b->last_write_time;
	});
}

long long PdfCreator::GetLastWriteTime(const wstring& path)
{
	filesystem::file_time_type lastWriteTime = filesystem::last_write_time(path);
	auto lastTime = lastWriteTime.time_since_epoch();
	return chrono::duration_cast<std::chrono::milliseconds>(lastTime).count();
}

void PdfCreator::ClearPngList()
{
	for (auto info : m_vecPng)
		delete info;
	m_vecPng.clear();
}
