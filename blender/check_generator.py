# Run from blender, as:
#   blender -b blend_file_to_test.blend -P check_generator.py
#
# This is a test script for checking whether a .blend is suitable
# for use as a generator on live.makesweet.com
#
# Author:  Paul Fitzpatrick, paul-at-makesweet.com
# License: GPL v2 or later
#

import Blender
import re
import os
import glob
import sys

warnings = 0
errors = 0

# Check for controllable images.  These are images that are supposed
# to be changed by the generator.  They are distinguished from images
# that should be left untouched by their name.  Controllable images
# should have the prefix "mkswt_".

print("="*75 + "\n= Controllable image check")
imgs = Blender.Image.Get()
imgs.sort(lambda x,y: cmp(x.name,y.name))
controllable = []
for img in imgs:
    if img.users>0:
        print("  (checking image '%s')" % img.name)
        if img.name.find("mkswt_")==0:
            print("Controllable image found: " + img.name)
            controllable.append(img.name)

if len(controllable)==0:
    print("ERROR: No controllable image found.  Please add an image whose ")
    print("name contains the sequence 'mkswt_'")
    errors = errors+1


# Check that all (non-controllable) images are packed.  Other resources
# such as fonts should be packed too, but this isn't checked here.

print("="*75 + "\n= Packed images check")
for img in Blender.Image.Get():
    print("  (checking image '%s')" % img.name)
    if not(img.packed) and img.filename!="" and not(img.name in controllable):
        print("WARNING: This image is not packed: " + img.name)
        warnings = warnings+1

# Check for the presence of unexpected scripts, and warn that they
# will be ignored.

print("="*75 + "\n= Script check")
for txt in Blender.Text.Get()[1:]:
    print("  (checking text block '%s')" % txt.name)
    if len("".join(txt.asLines()))>0:
        if txt.name!="mkswt_config":
            print("WARNING: If this object is a script, it will be ignored: " +
                  txt.name)
            warnings = warnings+1
        else:
            print("    (found makesweet config object '%s')" % txt.name)

# Check that the render size is set to 800x600.

print("="*75 + "\n= Render size check")
context = Blender.Scene.GetCurrent().getRenderingContext()
width = context.imageSizeX()
height = context.imageSizeY()

if width!=800 or height!=600:
    print("ERROR: Render size is currently set to %dx%d.  Only 800x600 is currently supported."%(width,height))
    errors = errors+1
else:
    print("  (correct size, %dx%d)" % (width,height))

# Summarize the outcome of testing.

print("="*75 + "\n= Summary")
if len(controllable)>0:
    print("Controllable image(s) found: %s" % ", ".join(controllable))
print("Errors: %d" % errors)
print("Warnings: %d" % warnings)
if errors>0 or warnings>0:
    print("Errors are always serious, warnings are just things to check.")

sys.exit(errors)
