#ifndef _FACTORYIMP_H_
#define _FACTORYIMP_H_
__interface IEinkuiSystem;



#define CONFIG_FILE_NAME_MAX_LEN 20 //配置文件名字长度

DECLARE_BUILTIN_NAME(CFactoryImp)
class CFactoryImp : public cmmBaseObject<CFactoryImp, IElementFactory, GET_BUILTIN_NAME(CFactoryImp)>
{
protected:
	CFactoryImp();
	virtual ~CFactoryImp();


	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	DEFINE_CUMSTOMIZE_CREATE(CFactoryImp, (), ())
public:

	// 从配置文件中创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN ICfKey* npTemplete,			// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

	// 通过类名，创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN const wchar_t*		nswClassName,	// 类名
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

	// 获得与此Module配套的Profile文件接口，返回的接口当不再使用时，需要Release
	virtual IConfigFile* __stdcall GetTempleteFile(void);

	// 获得本DLL唯一的工厂对象
	static CFactoryImp* GetUniqueObject(void);

protected:
	// 唯一实例
	static CFactoryImp* gpFactoryInstance;
	IConfigFile* mpConfig;

	CMapList<AFX_MAPCALL, CHashFront>	moMapList;

};




























#endif//_FACTORYIMP_H_