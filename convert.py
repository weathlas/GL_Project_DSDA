from PIL import Image
import numpy
import matplotlib.image as img

im_rgb = Image.open('assets/textures/frosted.jpg')

im_rgba = im_rgb.copy()
im_rgba.putalpha(128)

data = numpy.asarray(im_rgba)

data_ok = numpy.copy(data)

print(data_ok.shape)

for y in range(data_ok.shape[0]):
    print("line", y)
    for x in range(data_ok.shape[1]):

        r, g, b, a = data_ok[y][x]

        data_ok[y][x] = (r*1.1, g*1.3, b*1.6, r)
        # data_ok[y][x][3] = data_ok[y][x][0]
        # data_ok[y][x][0] = data_ok[y][x][0]*1.1
        # data_ok[y][x][1] = data_ok[y][x][1]*1.3
        # data_ok[y][x][2] = data_ok[y][x][2]*1.6

img.imsave(f"assets/textures/frosted.png", data_ok)


# im_rgba.
# im_rgba.save('assets/textures/GlassFinalAlpha.png')
