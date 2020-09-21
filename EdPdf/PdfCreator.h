#pragma once

#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

#include <string>
#include <vector>

using namespace std;

namespace Yoga2
{
	struct FileInfo
	{
		string name;
		string path;
		long long last_write_time = 0;

		FileInfo(const string& _name, const string& _path, long long& time) :
			name(_name), path(_path), last_write_time(time)
		{}
	};

	class PdfCreator
	{
	public:
		PdfCreator();
		~PdfCreator();

		void AddImageToPdf(const wstring& pngPath);
		void SavePdf(const wstring& pdfPath);

	private:
		void CreatePage(const string& name, const string& path);
		string WideStr2UTF8Str(const std::wstring& wideStr);
		void LoadPngList(const wstring& path);
		long long GetLastWriteTime(const wstring& path);
		void ClearPngList();

	private:
		fz_context* m_pCtx = nullptr;
		pdf_document *m_pDoc = nullptr;
		pdf_write_options m_Opts;

		vector<FileInfo*> m_vecPng;
	};
}
