### MarkdownGS
---
##### By Chris Vavruska
##### Copyright (C) 2021

### Introduction

MarkdownGS is a translator module for Babelfish by Seven Hils Systems (http://speccie.uk/software/babelfish) that converts markdown text formats to other Babelfish supported formats. Markdown is light-weight markup language for creating formatted text using a plain text-editor. Babelfish is an NDA (New Desk Accessory) for the the Apple IIgs System 6+ which allows the convertion from one compatible file format to another (text->text, graphic->graphic, etc). MarkdownGS acts as both an import translator as well as an export translator so it is now possible to ediit files on you applie IIGS and display them on other computers as they were meant to be seen.

### Goals

The main goal of MarkdownGS is to allow the importing and exporting of markdown formatted files and have the output look the same as the original intentded it to look. Unfortunately, due to the limited functionality of the Apple IIgs toolset that goal can not be 100% completed. What can be completed is something that is as close we can come to as a reproduction of the original.
MarkdownGS supports all the basic syntax of the markdown language except for the image and link elements.

### Installation

To install MarkdownGS copy the MarkdownGS translator file to the :System:SSH.Babelfish folder of your boot volume and reboot. You must have previously installed the Babelfish NDA along with its other supported translators.

### Using MarkdownGS

MarkdownGS requires a Babelfish friendly application to invoke the functionality of MarkdownGS.  Fortunately Babelfish comes with a "Convert" NDA which will allow a user to choose and import file in one format and export thecontents to another format. Since Markdown is a text file format then the source and destination file formats must be text formats. Specturm(tm) by Ewen Wannop can also be used to import and export text file formats.  You can also use my ORCA shell utility (https://github.com/vavruska/babelfishCLI to convert files. 
See the application software documentation for how to use the Babelfish functionality.

### Importing

The Markdown language does not include any sort of font information, such as font face or font size. Instead it refers to sizes as header 1,2,3,4,5,6 or normal text. In order for MarkdownGS to import a markdown file you must tell it what font size on the Apple IIGS to use when it sees particular markdown size. The default font sizes are 36,30,27,24,20,18 for the headers 1-6 and 12 for normal text. The default font face is Helvetica. 

##### Import Options

Using the Import Options button from the Babelfish import file dialog you can changes the defaults.

**Use SIS Fonts for defaults **- If you have the SIS fonts installed then you can choose to use those as the default. The SIS fonts have been optimized for usage on the Apple IIgs using smaller font sizes. When using the SIS fonts you can not change the font sizes.

**Text Font** - Change the font used when converting a markdown file to a GS specific formatted file.

**Font Sizes** - Change the font size used when displaying a header or normal text.

**Indent Style** - Certain Markdown elements cause the text to be indented. The default is to use 8 spaces for indents but you can choose to use a single tab, as well 2,4,6 or 8 spaces. 

### Exporting

Like importing, exported must know what font sizes found in a document should be converted to the appropriate markdown header size.  All font sizes that do not match an export header size will match as normal  text.

##### Export Options

Using the "Export Options" dialog you can choose which font size convert to a particular markdown header size. 

**Show Export Translate Window** - The Babelfish status dialog is only shown during the import phase. MarkdownGS exports it two phases. The first phase is to gather all the text and style data from the original file. The second phase is to write out the data using the markdown formatting. After importing is completed the status dialog is closed. Enabling this option (default) shows a status dialog which displays the progress of the second phase of exporting.

**Use Same Font Sizes as Importing** - Enabling this option (default) tells to use the same font sizes that are defined in the import settings. 

**Font Sizes** - Defined the font sizes that are used for the header sizes.

**End of Line Format **- Different operating systems use different character or combination of character for their line ending. Using this option you can select which line ending you should use when exporting files. The choices are Apple (default), Linux/Unix or Windows.

**Indent Detect Count **- The number of spaces entered at a start of a line to signify an indent. Used when using mulitple levels of lists. Mutiples of this count is used for multiple levels. Also used for block quotes.

#### Preparing a document for exporting

When you import a Markdown document the formatting is pretty straight forward and can easily be converted when importing a document.  Exporting is not as straight forward. Here are some things you need to do to make sure your file exports as you expect it to.

##### Font Sizes

Font size changes are not always easily seen and a rogue font size change can produce a Markdown file that has a different apperance even when re-importing back into a GS formated file. While exporting if a font size is detected that does not match one of the heading font sizes then it will be treated as normal text. Be sure to only use the font sizes that are defined as the header sizes.

##### Lists

Ordered and unordered lists can start on column zero of a document but in order to detect an indent it must contains a single tab or the correct number of spaces. The correct number of spaces can be chosen in the Babelfish Export options. The correct number of spaces defaults to 8 as it seems to work well with the Helvetica font. 

##### Block Quotes

In order for text to be detected as a block quote it must start with the correct number of spaces and have a background color of yellow. The yellow is used in place of the verticle bars uses on markdown viewers on other platforms.

##### Code/Code Blocks

Text that is using the monospaces fonts Courier or SIS3 and a have yellow background will be treated as a code with entire lines being treated as a code block. Code may not have any other font style changes.

##### What does MarkdownGS understand?

Since most Apple IIGS editors use the Text Edit tool for displaying and editing text not all styles and text formating are available. Markdown GS can import and export the following formats types

# Header 1
## Header 2
### Header 3
#### Header 4
##### Header 5
###### Header 6

**Bold Text**

*Italic Text*

##### Horizontal lines (74 underscores)
---

##### Ordered Lists

1. Item 1
2. Item 2
    1. Sub Item 1
    2. Sub Item 2
3. Item 3

##### Unordered Lists
- Item 1
    - Sub Item 1a
    - Sub Item 1b
- Item 2

##### Block Quotes
> Blockquote Text must have a yellow background
>> More Block quote 8 spaces to create another nest
> 
> More text after a blank

##### Inline Code 

This is an `inline code`

##### Code Blocks
```
   Code is defined as text using the courier font (or the SIS3 font)
   with a white font color with a black background to make it stand out.
   Code blocks end when the font face changes to a non-courier font


```
#### Open Source Notice

MarkdownGS is open source and can be found at https://github.com/vavuska/markdownGS.

MarkdownGS includes the open source tool md4c (http://github.com/mity/md4c) to handle the importing of markdown formatted files.

#### Building MarkdownGS from source

MarkdownGS was developed in a Linux environment using the GSplus emulator and the GoldenGate Apple IIgs compatibility layer by Kelvin Sherlock (https://juiced.gs/store/golden-gate/). It includes a Makefile and some tinkering will be needed to build on other operating systems.  
Unforutunately, MarkdownGS can not be built on an Apple IIgs due to an artificial 32K limitation inside the resource compliler. If someone were to fix this limitation or create then use the included ORCA shell file build.

#### Thanks

Thanks go out to Jeremy Rand who's original version of md2teach (https://github.com/jeremysrand/md2teach) inspired me to write the MarkdownGS translator and also to release it as open source.

---
End of readme