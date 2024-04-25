from PIL import Image
import os


ImageName = "Images/Kreis_Vario.jpg"


def calculate_gray(r, g, b):
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def main():
    script_dir = os.path.dirname(__file__)  # <-- absolute dir the script is in
    abs_file_path = os.path.join(script_dir, ImageName)
    try:
        image = Image.open(open(abs_file_path, 'rb'))
    except IOError:
        print("Error loading image.")
        return

    width, height = image.size
    print(f"Image dimensions: {width} x {height}")
    scale = float(input("Enter scale for Image (0-1): "))
    image.thumbnail(
        tuple(element * scale for element in image.size), Image.Resampling.LANCZOS)

    width, height = image.size
    print(f"New image dimensions: {width} x {height}")

    outputName = input("Enter output Arrayname: ")

    widthInBytes = width / 8 + 1
    if width % 8 == 0:
        widthInBytes = width / 8
    array = bytearray(int(widthInBytes * height))
    for x in range(width):
        for y in range(height):
            r, g, b = image.getpixel((x, y))
            gray = calculate_gray(r, g, b)
            if gray < 230:
                array[int(x / 8 + y * widthInBytes)] = (
                    array[int(x / 8 + y * widthInBytes)] & ~(0x80 >> (x % 8)))
            else:
                array[int(x / 8 + y * widthInBytes)] = (
                    array[int(x / 8 + y * widthInBytes)] | (0x80 >> (x % 8)))

    image.close()


if __name__ == "__main__":
    main()
