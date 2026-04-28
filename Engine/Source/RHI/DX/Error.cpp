#include "RHI/DX/Error.h"

namespace ZE::RHI::DX
{
	std::string Error::message(int condition) const
	{
		switch (static_cast<HRESULT>(condition))
		{
		case DEBUG_MSG_ERROR:
			return "DirectX Debug Layer reported errors after call!";
		case ALLOC_ERROR:
			return "Failed to allocate GPU memory!";
		case NO_ADAPTER_ERROR:
			return "Cannot find suitable adapter for device creation!";
		case ENHANCED_BARRIERS_ERROR:
			return "Enhanced Barriers not supported by current driver!";
		case DSTORAGE_REQUEST_FAILURE:
			return "The DirectStorage request failed!";
		case DISKMANAGER_INVALID_HANDLE:
			return "Unknown DiskStatusHandle handle to wait for!";
		default:
			return Platform::WinAPI::Error::message(condition);
		}
	}
}