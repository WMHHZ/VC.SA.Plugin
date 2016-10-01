#ifdef XLIVELESS_EXPORTS
#define XLIVELESS_API extern "C" __declspec(dllexport)
#else
#define XLIVELESS_API extern "C" __declspec(dllimport)
#endif

#define XLIVELESS_VERSION   0x00010000  // 1.0.0

enum GameVersion {
    GvUnknown       = 0x00000000,   // unknown game
    IvPatch1        = 0x00010001,   // GTA IV 1.0.1.0
    IvPatch2        = 0x00010002,   // GTA IV 1.0.2.0
    IvPatch3        = 0x00010003,   // GTA IV 1.0.3.0
    IvPatch4        = 0x00010004,   // GTA IV 1.0.4.0
    IvPatch5        = 0x00010005,   // GTA IV 1.0.0.4
    IvPatch6        = 0x00010006,   // GTA IV 1.0.6.0
    IvPatch6J       = 0x00010006,   // GTA IV 1.0.4.2
    IvPatch7        = 0x00010007,   // GTA IV 1.0.7.0
    RfgUnpatched    = 0x00020000,   // Red Faction: Guerilla
    EflcPatch1      = 0x00030001,   // EfLC 1.1.1.0
    EflcPatch2      = 0x00030001,   // EfLC 1.1.2.0
};

// for C/C++ plugins
XLIVELESS_API GameVersion   dwGameVersion;  // game 
XLIVELESS_API DWORD         dwLoadOffset;   // Offset to the "real" loding address 

// for Delphi plugins (same as two variables above)
XLIVELESS_API GameVersion   getGameVersion();
XLIVELESS_API DWORD         getLoadOffset ();

// Print message to the log 
XLIVELESS_API void trace(char * message, ...);

// Replace game function at dwAddress with plugin function (cast pointer to the function/method to DWORD)
XLIVELESS_API void injectFunction (DWORD dwAddress, DWORD pfnReplacement);
