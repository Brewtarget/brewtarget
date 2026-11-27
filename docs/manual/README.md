# User Manual

This directory holds the source of the user manual, written in [AsciiDoc](https://docs.asciidoctor.org/asciidoc/latest/).
It was previously maintained in the separate [Brewtarget/manual](https://github.com/Brewtarget/manual) repository.

The manual is licensed under the GNU Free Documentation License, version 1.3 or later -- see `COPYING.FDL_1.3`, and
the `Files: docs/manual/*` section of the top-level `COPYRIGHT` file -- rather than the GPL that covers the rest of the
code.  If you contribute to the manual, add yourself to that section.

## Layout

```
docs/manual/
  en/                 English manual -- this is the "gold standard" that other languages follow
    book.adoc         Top-level file: document attributes, title, and the list of included chapters
    README.adoc       Contributors, foreword and preface
    chapters/         One file per chapter
    assets/           Screenshots and other images
    license.adoc      Text of the GFDL
  fr/                 French translation (incomplete, and from an older version of the manual -- not currently built)
```

Write `{app-name}` rather than "Brewken" or "Brewtarget" in the text, so that the same source works for both.  The
other attributes you can use are `{app-version}` and `{app-github-url}`.  Defaults are set at the top of
`en/book.adoc`; the builds below override them.

If you add a chapter, include it from `en/book.adoc` **and** add it to `manualSources` in `meson.build`.

## Building

### PDF (shipped with the software)

If [Asciidoctor PDF](https://docs.asciidoctor.org/pdf-converter/latest/) is installed, the normal Meson build creates
`manual-en.pdf` in the build directory, and `meson install` / `bt package` installs it with the program's data files.
About > Manual opens this PDF, or, if it's not there, the online manual.

`bt setup all` tries to install Asciidoctor PDF but carries on without it if it can't.  On Ubuntu you can install it
with `sudo apt install ruby-asciidoctor-pdf`; elsewhere, `gem install asciidoctor-pdf`.

To rebuild just the manual: `meson compile -C mbuild manual`

### HTML (online)

`.github/workflows/manual.yml` builds the HTML and PDF and publishes them to GitHub Pages when a release is published.
There is only one online manual, for the latest release -- older releases have their own manual in the PDF they ship
with.  You can also run the workflow by hand to republish (eg after fixing a mistake), but choose the release tag when
you do, otherwise the online manual may describe features that haven't been released yet.  See the comments at the top
of that file for the one-off repository setup.

To preview the HTML locally:

```bash
asciidoctor --attribute app-name=Brewken --destination-dir /tmp/manual --out-file index.html docs/manual/en/book.adoc
cp -r docs/manual/en/assets /tmp/manual/
xdg-open /tmp/manual/index.html
```

## Contributing

Keep changes focused, and check spelling and grammar.  Major additions and rewrites should be made in the English
manual first and then translated.

When a change to the program alters what the user sees, please update the manual in the same pull request.
