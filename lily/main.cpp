#pragma once
#include <stdio.h>
#include <Shlobj.h>

#include "common/dbvm.h"
#include "common/process.h"
#include "common/encrypt_string.h"
#include "common/render_ddraw.h"

#include "global.h"
#include "kernel_lily.h"
#include "render_lily.h"
#include "hack.h"
#include "dump.h"

void realmain() {
#ifdef DPRINT
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON"e, "w"e, stdout);
	dprintf("debug print enabled"e);
#endif
	if (IsUserAnAdmin())
		dprintf("Admin"e);
	else
		dprintf("Not Admin"e);

	KernelLily kernel(Global::dbvm);
	dprintf("Kernel OK"e);

#ifdef _WINDLL
	RenderLily render(kernel, Hack::FONTSIZE_SMALL);
#else
	RenderDDraw render(Hack::FONTSIZE_SMALL);
#endif

	for (;; Sleep(1)) {
		render.RenderArea(FindWindowA("Progman"e, "Program Manager"e), Render::COLOR_CLEAR, [&] {
			render.DrawString({ 500.0f, 0.0f, 0.0f }, Hack::MARGIN, "Waiting for PUBG..."e, Hack::FONTSIZE_BIG, Render::COLOR_RED, true, true, true);
			});

		const HWND hGameWnd = FindWindowA("UnrealWindow"e, 0);
		if (!hGameWnd)
			continue;

		//Process process(kernel, GetPIDFromHWND(hGameWnd));
		////DumpAll(process);
		//Dump(process);
		//exit(0);

		ExceptionHandler::TryExcept([&] {
			PubgProcess pubg(kernel, hGameWnd);
			Hack hack(pubg, render, kernel);
			hack.Loop();
			});
	}
}