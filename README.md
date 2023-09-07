# RadeonGraphicDriverBugExample

This project was created to reproduce a bug in the Radeon OpenGL Graphic Driver.
The bug is believed to occur on GPUs using the latest RDNA3 architecture.
The condition to reproduce it is to attach two textures in RGBA16 format to an FBO and then access SrcAlpha via glBlendFuncSeparatei(1, ..), which causes an undefined behavior.
