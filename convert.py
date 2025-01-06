from PIL import Image, ImageDraw, ImageFilter
import numpy
import matplotlib.image as img

im_rgb = Image.open('assets/textures/terrain_color_01.jpg')

im_rgba = im_rgb.copy()
# im_rgba.putalpha(128)

data = numpy.asarray(im_rgba)

data_ok = numpy.copy(data)

print(data_ok.shape)

for y in range(data_ok.shape[0]):
    for x in range(data_ok.shape[1]):
        data_ok[y][x][3] = 1

img.imsave(f"assets/textures/terrain.png", data_ok)


# im_rgba.
# im_rgba.save('assets/textures/GlassFinalAlpha.png')
