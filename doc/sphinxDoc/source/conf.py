# -*- coding: utf-8 -*-
#
# XC documentation build configuration file, created by
# sphinx-quickstart on Fri May 27 07:42:59 2016.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

import sys
import os
import re

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#sys.path.insert(0, os.path.abspath('.'))
pth_materials= '../../python_modules/materials'
sys.path.insert(0,os.path.abspath(pth_materials))
pth_cpplib= '../../lib'
sys.path.insert(0,os.path.abspath(pth_cpplib))

#boost.python stuff
#Copied from: http://bazaar.launchpad.net/~eudoxos/minieigen/trunk/view/head:/doc/source/conf.py
def isBoostFunc(what,obj):
	return what=='function' and obj.__repr__().startswith('<Boost.Python.function object at 0x')
def isBoostMethod(what,obj):
	"I don't know how to distinguish boost and non-boost methods..."
	return what=='method' and obj.__repr__().startswith('<unbound method ')
def isBoostStaticMethod(what,obj):
	return what=='method' and obj.__repr__().startswith('<Boost.Python.function object at 0x')

def fixDocstring(app,what,name,obj,options,lines):
	if isBoostFunc(what,obj) or isBoostMethod(what,obj) or isBoostStaticMethod(what,obj):
		l2=boostFuncSignature(name,obj)[1]
		# we must replace lines one by one (in-place) :-|
		# knowing that l2 is always shorter than lines (l2 is docstring with the signature stripped off)
		for i in range(0,len(lines)):
			lines[i]=l2[i] if i<len(l2) else ''

def fixSignature(app, what, name, obj, options, signature, return_annotation):
	print what,obj
	if what in ('attribute','class'): return signature,None
	elif isBoostFunc(what,obj):
		sig=boostFuncSignature(name,obj)[0] or ' (wrapped c++ function)'
		return sig,None
	elif isBoostMethod(what,obj):
		sig=boostFuncSignature(name,obj,removeSelf=True)[0]
		return sig,None
	elif isBoostStaticMethod(what,obj):
		sig=boostFuncSignature(name,obj,removeSelf=False)[0]+' [STATIC]'
		return sig,None

def boostFuncSignature(name,obj,removeSelf=False):
	"""Scan docstring of obj, returning tuple of properly formatted boost python signature
	(first line of the docstring) and the rest of docstring (as list of lines).
	The rest of docstring is stripped of 4 leading spaces which are automatically
	added by boost.
	
	removeSelf will attempt to remove the first argument from the signature.
	"""
	doc=obj.__doc__
	if doc==None: # not a boost method
		return None,None
	nname=name.split('.')[-1]
	docc=doc.split('\n')
	if len(docc)<2: return None,docc
	doc1=docc[1]
	# functions with weird docstring, likely not documented by boost
	if not re.match('^'+nname+r'(.*)->.*$',doc1):
		return None,docc
	if doc1.endswith(':'): doc1=doc1[:-1]
	strippedDoc=doc.split('\n')[2:]
	# check if all lines are padded
	allLinesHave4LeadingSpaces=True
	for l in strippedDoc:
		if l.startswith('    '): continue
		allLinesHave4LeadingSpaces=False; break
	# remove the padding if so
	if allLinesHave4LeadingSpaces: strippedDoc=[l[4:] for l in strippedDoc]
	for i in range(len(strippedDoc)):
		# fix signatures inside docstring (one function with multiple signatures)
		strippedDoc[i],n=re.subn(r'([a-zA-Z_][a-zA-Z0-9_]*\() \(object\)arg1(, |)',r'\1',strippedDoc[i].replace('->','→'))
	# inspect dosctring after mangling
	sig=doc1.split('(',1)[1]
	if removeSelf:
		# remove up to the first comma; if no comma present, then the method takes no arguments
		# if [ precedes the comma, add it to the result (ugly!)
		try:
			ss=sig.split(',',1)
			if ss[0].endswith('['): sig='['+ss[1]
			else: sig=ss[1]
		except IndexError:
			# grab the return value
			try:
				sig=') -> '+sig.split('->')[-1]
			except IndexError:
				sig=')'
	return '('+sig,strippedDoc

# def setup(app):
# 	app.connect('autodoc-process-docstring',fixDocstring)
# 	app.connect('autodoc-process-signature',fixSignature)

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix of source filenames.
source_suffix = '.rst'

# The encoding of source files.
#source_encoding = 'utf-8-sig'
source_encoding = 'utf-8'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'XC'
copyright = u'2016, Luis Pérez Tato, Ana Ortega'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = '3.0'
# The full version, including alpha/beta/rc tags.
release = '3.0'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#language = None

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
#today_fmt = '%B %d, %Y'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
exclude_patterns = []

# The reST default role (used for this markup: `text`) to use for all
# documents.
#default_role = None

# If true, '()' will be appended to :func: etc. cross-reference text.
#add_function_parentheses = True

# If true, the current module name will be prepended to all description
# unit titles (such as .. function::).
#add_module_names = True

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
#show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# A list of ignored prefixes for module index sorting.
#modindex_common_prefix = []

# If true, keep warnings as "system message" paragraphs in the built documents.
#keep_warnings = False


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'default'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#html_theme_options = {}

# Add any paths that contain custom themes here, relative to this directory.
#html_theme_path = []

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
#html_title = None

# A shorter title for the navigation bar.  Default is the same as html_title.
#html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#html_logo = None

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
#html_favicon = None

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Add any extra paths that contain custom files (such as robots.txt or
# .htaccess) here, relative to this directory. These files are copied
# directly to the root of the documentation.
#html_extra_path = []

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
#html_last_updated_fmt = '%b %d, %Y'

# If true, SmartyPants will be used to convert quotes and dashes to
# typographically correct entities.
#html_use_smartypants = True

# Custom sidebar templates, maps document names to template names.
#html_sidebars = {}

# Additional templates that should be rendered to pages, maps page names to
# template names.
#html_additional_pages = {}

# If false, no module index is generated.
#html_domain_indices = True

# If false, no index is generated.
#html_use_index = True

# If true, the index is split into individual pages for each letter.
#html_split_index = False

# If true, links to the reST sources are added to the pages.
#html_show_sourcelink = True

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
#html_show_sphinx = True

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
#html_show_copyright = True

# If true, an OpenSearch description file will be output, and all pages will
# contain a <link> tag referring to it.  The value of this option must be the
# base URL from which the finished HTML is served.
#html_use_opensearch = ''

# This is the file name suffix for HTML files (e.g. ".xhtml").
#html_file_suffix = None

# Output file base name for HTML help builder.
htmlhelp_basename = 'XCdoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
#'papersize': 'letterpaper',

# The font size ('10pt', '11pt' or '12pt').
#'pointsize': '10pt',

# Additional stuff for the LaTeX preamble.
#'preamble': '',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
  ('index', 'XC.tex', u'XC Documentation',
   u'LCPT A\\_OO', 'manual'),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
#latex_logo = None

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
#latex_use_parts = False

# If true, show page references after internal links.
#latex_show_pagerefs = False

# If true, show URL addresses after external links.
#latex_show_urls = False

# Documents to append as an appendix to all manuals.
#latex_appendices = []

# If false, no module index is generated.
#latex_domain_indices = True


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('index', 'xc', u'XC Documentation',
     [u'LCPT A_OO'], 1)
]

# If true, show URL addresses after external links.
#man_show_urls = False


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
  ('index', 'XC', u'XC Documentation',
   u'LCPT A_OO', 'XC', 'One line description of project.',
   'Miscellaneous'),
]

# Documents to append as an appendix to all manuals.
#texinfo_appendices = []

# If false, no module index is generated.
#texinfo_domain_indices = True

# How to display URL addresses: 'footnote', 'no', or 'inline'.
#texinfo_show_urls = 'footnote'

# If true, do not generate a @detailmenu in the "Top" node's menu.
#texinfo_no_detailmenu = False