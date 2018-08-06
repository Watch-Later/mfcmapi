#pragma once
#include <Interpret/SmartView/SmartViewParser.h>

namespace smartview
{
	struct FolderFieldDefinitionCommon
	{
		blockT<GUID> PropSetGuid;
		blockT<DWORD> fcapm;
		blockT<DWORD> dwString;
		blockT<DWORD> dwBitmap;
		blockT<DWORD> dwDisplay;
		blockT<DWORD> iFmt;
		blockT<WORD> wszFormulaLength;
		blockT<std::wstring> wszFormula;
	};

	struct FolderFieldDefinitionA
	{
		blockT<DWORD> FieldType;
		blockT<WORD> FieldNameLength;
		blockT<std::string> FieldName;
		FolderFieldDefinitionCommon Common;
	};

	struct FolderFieldDefinitionW
	{
		blockT<DWORD> FieldType;
		blockT<WORD> FieldNameLength;
		blockT<std::wstring> FieldName;
		FolderFieldDefinitionCommon Common;
	};

	class FolderUserFieldStream : public SmartViewParser
	{
	public:
		FolderUserFieldStream();

	private:
		void Parse() override;
		void ParseBlocks() override;

		FolderFieldDefinitionCommon BinToFolderFieldDefinitionCommon();

		blockT<DWORD> m_FolderUserFieldsAnsiCount;
		std::vector<FolderFieldDefinitionA> m_FieldDefinitionsA;
		blockT<DWORD> m_FolderUserFieldsUnicodeCount;
		std::vector<FolderFieldDefinitionW> m_FieldDefinitionsW;
	};
}