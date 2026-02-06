#include "RHI/DX/Error.h"

namespace ZE::RHI::DX
{
	std::string Error::message(int condition) const
	{
		if (static_cast<HRESULT>(condition) == DEBUG_MSG_ERROR)
			return "DirectX Debug Layer reported errors after call!";
		return Platform::WinAPI::Error::message(condition);
	}
}