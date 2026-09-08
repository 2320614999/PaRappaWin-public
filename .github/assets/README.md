# Presentation assets

[English home](../../README.md) · [中文首页](../../README.zh-CN.md)

These assets give the public repository a consistent visual identity. They are
original geometric/typographic project decoration, not screenshots, official
cover art, a statement of endorsement, or evidence of gameplay parity. No game
characters, extracted textures or private media were used to make them.

| File | Purpose | Canvas |
| --- | --- | --- |
| [hero.svg](hero.svg) | Embedded README header, independent of third-party badge services | 1280 × 480 |
| [social-preview.svg](social-preview.svg) | Editable source for a share-card image | 1280 × 640 |

The palette is ink `#101d25`, mint `#78e3c5`, warm orange `#ff9867`, and cream
`#f3f0e6`. SVGs use local system font fallbacks; no fonts, scripts, remote fonts,
external images or tracking are embedded. Update the English and Chinese
homepages together when changing the presentation.

## Social preview activation

Adding an image to Git does **not** set GitHub's repository social-preview
setting. That is a separate repository setting, and has not been changed by the
README refresh.

Export `social-preview.svg` to an opaque 1280 × 640 PNG/JPG under 1 MB, then use
repository **Settings → Social preview → Edit → Upload an image** with an
account that has the necessary settings access. A PNG export accompanies the
homepage delivery. The SVG is the editable source, not a format to upload into
that setting.

GitHub's official [social-preview instructions](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/customizing-your-repositorys-social-media-preview)
describe the supported formats and sizing. You can also export locally with
CairoSVG installed: `cairosvg social-preview.svg -o social-preview.png`.

## Public boundary

These presentation assets do not change the source publication inventory,
build scripts, runtime behavior, development pause, or validation record.
Existing third-party materials retain their existing rights and notices; see
[PUBLIC_BOUNDARY.md](../../PUBLIC_BOUNDARY.md) and
[THIRD_PARTY.md](../../THIRD_PARTY.md).
