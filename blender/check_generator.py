#
# This is a test script for checking whether a .blend is suitable
# for use as a generator on live.makesweet.com
#
# Run from blender, as:
#   blender -b blend_file_to_test.blend -P check_generator.py
#
# Author:  Paul Fitzpatrick <paul@makesweet.com>
# License: GPL v2 or later
#

import Blender
import re
import os
import glob

warnings = 0
errors = 0

print("="*75 + "\n= Script check")
txtct = 0
for txt in Blender.Text.Get():
    if txtct>0:
        print("WARNING: If this object is a script, it will be ignored: " + txt.name)
        warnings = warnings+1
    txtct = txtct+1

print("="*75 + "\n= Packed images check")
for img in Blender.Image.Get():
    if ((not(img.packed)) and img.filename!=""):
        print("WARNING: This image is not packed: " + img.name)
        warnings = warnings+1

print("="*75 + "\n= Material check")
haveMaterial = False
for mat in Blender.Material.Get():
    enabledIndices = mat.enabledTextures
    at = 0
    textures = mat.getTextures()
    for mtex in textures:
        if at in enabledIndices and not(mtex is None):
            name = mtex.tex.name
            print("  (checking material '%s', texture '%s')" % (mat.name, name))
            if (name.find("mod_")==0):
                if mtex.tex.type == Blender.Texture.Types.IMAGE:
                    print("Active texture found, '" + name +
                          "' attached to material '" + mat.name + "'")
                    haveMaterial = True
        at = at+1

if not(haveMaterial):
    print("ERROR: No active material found.  Please add a material that has an ")
    print("image texture, with the image texture's name starting with mod_")
    errors = errors+1

print("="*75 + "\n= Render size check")
context = Blender.Scene.GetCurrent().getRenderingContext()
width = context.imageSizeX()
height = context.imageSizeY()

if width!=800 or height!=600:
    print("ERROR: Render size is currently set to %dx%d.  Only 800x600 is currently supported."%(width,height))
    errors = errors+1

print("="*75 + "\n= Test image")

if errors>0:
    print("Skipping generation of test images until errors fixed.")
else:
    for old_result in glob.glob("test_render_*.png"):
        os.rename(old_result,old_result + ".bak")
    for mat in Blender.Material.Get():
        enabledIndices = mat.enabledTextures
        at = 0
        textures = mat.getTextures()
        for mtex in textures:
            if at in enabledIndices and not(mtex is None):
                name = mtex.tex.name
                if (name.find("mod_")==0):
                    if mtex.tex.type == Blender.Texture.Types.IMAGE:
                        print("Replacing image '" + name +
                              "' attached to material '" + mat.name + "'")
                        # black test image
                        mtex.tex.image = Blender.Image.New("test_image",
                                                           500,500,32)
            at = at+1

    Blender.Scene.Render.EnableDispWin()
    context = Blender.Scene.GetCurrent().getRenderingContext()
    context.setRenderPath(os.path.join(os.getcwd(),"test_render_"))
    context.setImageType(Blender.Scene.Render.PNG)
    curFrame = context.currentFrame()
    context.startFrame(curFrame)
    context.endFrame(curFrame)
    context.setRenderWinSize(25)
    context.renderAnim()
    result = glob.glob("test_render_*.png")
    if len(result)==0:
        print("ERROR: Test render could not be created")
        errors = errors+1
    else:
        print("Test render created successfully, %s" % result[0])
        print("Please check that the inserted black area covers full surface of interest.")


print("="*75 + "\n= Summary")
print("Errors: %d" % errors)
print("Warnings: %d" % warnings)
if errors>0 or warnings>0:
    print("Errors are always serious, warnings are just things to check.")

