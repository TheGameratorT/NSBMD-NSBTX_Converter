ASS to IMD
A CLI tool capable of converting Assimp compatible 3D Models to Nitro Intermediate Model.
Designed to remove the need of Nitro Plugins for MKDS Custom Tracks and Custom Karts.
Written by Ermelber with Gericom's help.

Usage:
        AssToImd INPUT [OPTIONS]

Options:
        -o/--output     fileName        Path to the output file.
        -m/--mag        scaleFactor     Scales by the model by a magnification factor.
        -f/--flipYZ                     Flips the Y and Z Axis.
        -r/--rot180                     Rotates the model by 180 degrees on the X Axis.
        -l/--light                      Enables Light 0 on all materials. Needed for shaded models.
        -v/--verbose                    Output warnings and other info.

Comes with N00b-proof Batch files. Just drag and drop your model onto on of them and it will convert the model for you.
DragCourseModel.bat is used to convert course models and skyboxes (applies the 0.0625 scale) while the other one is for Karts and other stuff that has shading. 

Changelog:

[0.2.4]
Open source release. This is now part of a Nitro G3D Tools. Moved Intermediate stuff to a library. No other changes probably.

[0.2.3]
The application now outputs in the correct decimal number format (e.g. 0.1132 instead of 0,1132).

[0.2.2]
Workaround for PNG textures (might still cause other issues).

[0.2.1]
Fixed UV to ST translation. It caused issues when a texture was being clamped or mirrored.