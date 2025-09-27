#pragma once

struct IDirect3DDevice9;

extern long (__stdcall* g_OriginalEndScene)(IDirect3DDevice9*);

long __stdcall HookedEndScene(IDirect3DDevice9* pDevice);
