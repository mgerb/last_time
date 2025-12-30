# /// script
# requires-python = ">=3.13"
# dependencies = ['pillow']
# ///

"""
This script will generate png icons based on the provided nerdfont.
This is only used for documentation in the readme.

usage: uv run generate_glyph_png.py /path/to/font <icon>
"""

import argparse
import pathlib
import sys

from PIL import Image, ImageDraw, ImageFont


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Render a font glyph to a PNG with 1px white stroke and black fill."
    )
    parser.add_argument("font_path", help="Path to the font file (e.g. .ttf, .otf).")
    parser.add_argument("char", help="Character to render (or pass --codepoint).")
    parser.add_argument("--size", type=int, default=64, help="Font size in pixels.")
    parser.add_argument("--padding", type=int, default=10, help="Padding in pixels.")
    parser.add_argument(
        "--output",
        default="glyph.png",
        help="Output PNG path (default: glyph.png).",
    )
    args = parser.parse_args()

    font = ImageFont.truetype(args.font_path, args.size)

    dummy = Image.new("RGBA", (1, 1), (0, 0, 0, 0))
    draw = ImageDraw.Draw(dummy)
    left, top, right, bottom = draw.textbbox(
        (0, 0), args.char, font=font, stroke_width=1
    )

    width = right - left + args.padding * 2
    height = bottom - top + args.padding * 2
    if width <= 0 or height <= 0:
        print("Error: computed glyph bounds are empty.", file=sys.stderr)
        return 2

    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.text(
        (args.padding - left, args.padding - top),
        args.char,
        font=font,
        fill=(0, 0, 0, 255),
        stroke_width=1,
        stroke_fill=(255, 255, 255, 255),
    )

    output_path = pathlib.Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    image.save(output_path, "PNG")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
