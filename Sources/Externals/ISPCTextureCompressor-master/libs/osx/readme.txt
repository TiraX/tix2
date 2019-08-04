This libispc_texcomp.a was generated hack by following this:
https://github.com/GameTechDev/ISPCTextureCompressor/issues/13

The provided xcode project is configured to generate a dylib file, however I'm interested in a static library.

I've tried changing the Mach-O type to "Static Library" but that didn't help, because there's still a dylib file generated.
I've tried renaming it to *.a static lib as the file was different size than before, so I thought maybe just the extension is incorrect, however after linking to it, I get undefined references.

How to compile for "Static Library"?

Edit:

I see that the "Other Linker Flags" setting is set to "build/.o", however in my case the files are generated in "Build/Intermediates/.o", but after changing the Other Linker Flags to the correct directory, there were still undefined references, because those files weren't being linked (that didn't help).
In the end I was able to fix the problem by manually drag and drop the "Build/Intermediates/*.o" files to the xcode project, as if they were C++ files (to make them listed in the project).
After that the generated static library got generated in full with all *.o files and functions linked correctly, and it works fine.