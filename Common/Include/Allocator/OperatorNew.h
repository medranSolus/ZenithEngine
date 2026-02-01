#pragma once

namespace ZE::Allocator
{
	// Auxiliary function to force usage of the operator new and delete replacements.
	// Need to be called at least once in the code included in the final executable to force
	// inclusion of this object file functions.
	void CheckNewReplacement(bool& status) noexcept;
}