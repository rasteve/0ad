#include "precompiled.h"

#include "lib/posix.h"
#include "lib/lib.h"
#include "Atlas.h"

//----------------------------------------------------------------------------
// Atlas (map editor) integration
//----------------------------------------------------------------------------

static void* const ATLAS_SO_UNAVAILABLE = (void*)-1;
static void* atlas_so_handle;


// free reference to Atlas UI SO (avoids resource leak report)
void ATLAS_Shutdown()
{
	// (avoid dlclose warnings)
	if(atlas_so_handle != 0 && atlas_so_handle != ATLAS_SO_UNAVAILABLE)
		dlclose(atlas_so_handle);
}


// return true if the Atlas UI shared object is available;
// used to disable the main menu editor button if not.
// note: this actually loads the SO, but that isn't expected to be slow.
//       call ATLAS_Shutdown at exit to avoid leaking it.
static bool ATLAS_IsAvailable()
{
	// first time: try to open Atlas UI shared object
	// postcondition: atlas_so_handle valid or == ATLAS_SO_UNAVAILABLE.
	if(atlas_so_handle == 0)
	{
		// since this SO exports a C++ interface, it is critical that
		// compiler options are the same between app and SO; therefore,
		// we need to go with the debug version in debug builds.
		// note: on Windows, the extension is replaced with .dll by dlopen.
#ifndef NDEBUG
		const char* so_name = "AtlasUI_d.so";
#else
		const char* so_name = "AtlasUI.so";
#endif
		// we don't care when relocations take place because this SO contains
		// very few symbols, so RTLD_LAZY or RTLD_NOW aren't needed.
		const int flags = RTLD_LOCAL;
		atlas_so_handle = dlopen(so_name, flags);
		// open failed (mostly likely SO not found)
		if(!atlas_so_handle)
			atlas_so_handle = ATLAS_SO_UNAVAILABLE;
	}

	return atlas_so_handle != ATLAS_SO_UNAVAILABLE;
}


enum AtlasRunFlags
{
	// used by ATLAS_RunIfOnCmdLine; makes any error output go through
	// DISPLAY_ERROR rather than a GUI dialog box (because GUI init was
	// skipped to reduce load time).
	ATLAS_NO_GUI = 1
};

// starts the Atlas UI.
static void ATLAS_Run(int argc, char* argv[], int flags = 0)
{
	// first check if we can run at all
	if(!ATLAS_IsAvailable())
	{
		if(flags & ATLAS_NO_GUI)
			DISPLAY_ERROR(L"The Atlas UI was not successfully loaded and therefore cannot be started as requested.");
		else
			DISPLAY_ERROR(L"The Atlas UI was not successfully loaded and therefore cannot be started as requested.");// TODO: implement GUI error message
		return;
	}

	void(*pStartWindow)(int argc, char* argv[]);
	*(void**)&pStartWindow = dlsym(atlas_so_handle, "_StartWindow");
	pStartWindow(argc, argv);
}


// starts the Atlas UI if an "-editor" switch is found on the command line.
// this is the alternative to starting the main menu and clicking on
// the editor button; it is much faster because it's called during early
// init and therefore skips GUI setup.
// notes:
// - GUI init still runs, but some GUI setup will be skipped since
//   ATLAS_IsRunning() will return true.
// - could be merged into CFG_ParseCommandLineArgs, because that appears
//   to be called early enough. it's not really worth it because this
//   code is quite simple and we thus avoid startup order dependency.
void ATLAS_RunIfOnCmdLine(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++)	// skip program name argument
	{
		if(!strcmp(argv[i], "-editor"))
		{
			// don't bother removing this param (unnecessary)

			ATLAS_Run(argc, argv, ATLAS_NO_GUI);
			break;
		}
	}
}










