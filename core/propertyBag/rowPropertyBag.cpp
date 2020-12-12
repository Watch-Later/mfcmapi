#include <core/stdafx.h>
#include <core/propertyBag/rowPropertyBag.h>
#include <core/mapi/mapiFunctions.h>
#include <core/mapi/mapiMemory.h>

namespace propertybag
{
	rowPropertyBag::rowPropertyBag(sortlistdata::sortListData* lpListData, bool bIsAB)
		: m_lpListData(lpListData), m_bIsAB(bIsAB)
	{
		if (lpListData)
		{
			m_cValues = lpListData->cSourceProps;
			m_lpProps = lpListData->lpSourceProps;
		}

		if (mapi::IsABObject(m_cValues, m_lpProps)) m_bIsAB = true;
	}

	propBagFlags rowPropertyBag::GetFlags() const
	{
		auto ulFlags = propBagFlags::None;
		if (m_bIsAB) ulFlags |= propBagFlags::AB;
		if (m_bRowModified) ulFlags |= propBagFlags::Modified;
		return ulFlags;
	}

	bool rowPropertyBag::IsEqual(const std::shared_ptr<IMAPIPropertyBag> lpPropBag) const
	{
		if (!lpPropBag) return false;
		if (GetType() != lpPropBag->GetType()) return false;

		const auto lpOther = std::dynamic_pointer_cast<rowPropertyBag>(lpPropBag);
		if (lpOther)
		{
			if (m_lpListData != lpOther->m_lpListData) return false;
			if (m_cValues != lpOther->m_cValues) return false;
			if (m_lpProps != lpOther->m_lpProps) return false;
			return true;
		}

		return false;
	}

	_Check_return_ HRESULT rowPropertyBag::GetAllProps(ULONG FAR* lpcValues, LPSPropValue FAR* lppPropArray)
	{
		if (!lpcValues || !lppPropArray) return MAPI_E_INVALID_PARAMETER;

		// Just return what we have
		*lpcValues = m_cValues;
		*lppPropArray = m_lpProps;
		return S_OK;
	}

	// Always returns a propval, even in errors
	_Check_return_ LPSPropValue rowPropertyBag::GetOneProp(ULONG ulPropTag)
	{
		const auto prop = PpropFindProp(m_lpProps, m_cValues, ulPropTag);
		if (prop) return prop;
		m_missingProp.ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_ERROR);
		m_missingProp.Value.err = MAPI_E_NOT_FOUND;
		return &m_missingProp;
	}

	// Concatenate two property arrays without duplicates
	// Entries in the first array trump entries in the second
	// Will also eliminate any duplicates already existing within the arrays
	_Check_return_ HRESULT ConcatLPSPropValue(
		ULONG ulVal1,
		_In_count_(ulVal1) LPSPropValue lpVal1,
		ULONG ulVal2,
		_In_count_(ulVal2) LPSPropValue lpVal2,
		_Out_ ULONG* lpulRetVal,
		_Deref_out_opt_ LPSPropValue* lppRetVal)
	{
		if (!lpulRetVal || !lppRetVal) return MAPI_E_INVALID_PARAMETER;
		if (ulVal1 && !lpVal1) return MAPI_E_INVALID_PARAMETER;
		if (ulVal2 && !lpVal2) return MAPI_E_INVALID_PARAMETER;
		*lpulRetVal = NULL;
		*lppRetVal = nullptr;
		auto hRes = S_OK;

		ULONG ulTargetArray = 0;
		ULONG ulNewArraySize = 0;
		LPSPropValue lpNewArray = nullptr;

		// Add the sizes of the passed in arrays
		if (ulVal2 && ulVal1)
		{
			ulNewArraySize = ulVal1;
			// Only count props in the second array if they're not in the first
			for (ULONG ulSourceArray = 0; ulSourceArray < ulVal2; ulSourceArray++)
			{
				if (!PpropFindProp(lpVal1, ulVal1, CHANGE_PROP_TYPE(lpVal2[ulSourceArray].ulPropTag, PT_UNSPECIFIED)))
				{
					ulNewArraySize++;
				}
			}
		}
		else
		{
			ulNewArraySize = ulVal1 + ulVal2;
		}

		if (ulNewArraySize)
		{
			// Allocate the base array - MyPropCopyMore will allocmore as needed for string/bin/etc
			lpNewArray = mapi::allocate<LPSPropValue>(ulNewArraySize * sizeof(SPropValue));
			if (lpNewArray)
			{
				if (ulVal1)
				{
					for (ULONG ulSourceArray = 0; ulSourceArray < ulVal1; ulSourceArray++)
					{
						if (!ulTargetArray || // if it's NULL, we haven't added anything yet
							!PpropFindProp(
								lpNewArray,
								ulTargetArray,
								CHANGE_PROP_TYPE(lpVal1[ulSourceArray].ulPropTag, PT_UNSPECIFIED)))
						{
							hRes = EC_H(mapi::MyPropCopyMore(
								&lpNewArray[ulTargetArray], &lpVal1[ulSourceArray], MAPIAllocateMore, lpNewArray));
							if (SUCCEEDED(hRes))
							{
								ulTargetArray++;
							}
							else
								break;
						}
					}
				}

				if (SUCCEEDED(hRes) && ulVal2)
				{
					for (ULONG ulSourceArray = 0; ulSourceArray < ulVal2; ulSourceArray++)
					{
						if (!ulTargetArray || // if it's NULL, we haven't added anything yet
							!PpropFindProp(
								lpNewArray,
								ulTargetArray,
								CHANGE_PROP_TYPE(lpVal2[ulSourceArray].ulPropTag, PT_UNSPECIFIED)))
						{
							// make sure we don't overrun.
							if (ulTargetArray >= ulNewArraySize)
							{
								hRes = MAPI_E_CALL_FAILED;
								break;
							}

							hRes = EC_H(mapi::MyPropCopyMore(
								&lpNewArray[ulTargetArray], &lpVal2[ulSourceArray], MAPIAllocateMore, lpNewArray));
							if (SUCCEEDED(hRes))
							{
								ulTargetArray++;
							}
							else
								break;
						}
					}
				}

				if (FAILED(hRes))
				{
					MAPIFreeBuffer(lpNewArray);
				}
				else
				{
					*lpulRetVal = ulTargetArray;
					*lppRetVal = lpNewArray;
				}
			}
		}

		return hRes;
	}

	_Check_return_ HRESULT rowPropertyBag::SetProp(LPSPropValue lpProp)
	{
		ULONG ulNewArray = NULL;
		LPSPropValue lpNewArray = nullptr;

		const auto hRes = EC_H(ConcatLPSPropValue(1, lpProp, m_cValues, m_lpProps, &ulNewArray, &lpNewArray));
		if (SUCCEEDED(hRes))
		{
			MAPIFreeBuffer(m_lpListData->lpSourceProps);
			m_lpListData->cSourceProps = ulNewArray;
			m_lpListData->lpSourceProps = lpNewArray;

			m_cValues = ulNewArray;
			m_lpProps = lpNewArray;
			m_bRowModified = true;
		}

		return hRes;
	}

	_Check_return_ std::vector<std::shared_ptr<model::mapiRowModel>> rowPropertyBag::GetAllModels()
	{
		return model::propsToModels(m_cValues, m_lpProps, nullptr, m_bIsAB);
	}

	_Check_return_ std::shared_ptr<model::mapiRowModel> rowPropertyBag::GetOneModel(ULONG ulPropTag)
	{
		const SPropValue* lpPropVal = PpropFindProp(m_lpProps, m_cValues, ulPropTag);
		if (lpPropVal) return model::propToModel(lpPropVal, ulPropTag, nullptr, m_bIsAB);

		auto propVal = SPropValue{CHANGE_PROP_TYPE(ulPropTag, PT_ERROR), 0};
		propVal.Value.err = MAPI_E_NOT_FOUND;
		return model::propToModel(&propVal, ulPropTag, nullptr, m_bIsAB);
	}
} // namespace propertybag
