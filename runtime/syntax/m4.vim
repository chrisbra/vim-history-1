" Vim syntax file
" Language:     M4
" Maintainer:   Claudio Fleiner
" URL:		http://www.fleiner.com/vim/syntax/m4.vim
" Last change:  1999 Apr 22

" This file will highlight user function calls if they use only
" capital letters and have at least one argument (i.e. the '('
" must be there). Let me know if this is a problem.

" we define it here so that included files can test for it
if !exists("main_syntax")
  syn clear
  let main_syntax='m4'
endif

" define the m4 syntax
syn match  m4Variable contained "\$\d\+"
syn match  m4Special  contained "$[@*#]"
syn match  m4Comment  "dnl\>.*"
syn match  m4Constants "\(\<m4_\)\=__file__"
syn match  m4Constants "\(\<m4_\)\=__line__"
syn keyword m4Constants divnum sysval m4_divnum m4_sysval
syn region m4Paren    matchgroup=m4Delimiter start="(" end=")" contained contains=@m4Top
syn region m4Command  matchgroup=m4Function  start="\<\(m4_\)\=\(define\|defn\|pushdef\)(" end=")" contains=@m4Top
syn region m4Command  matchgroup=m4Preproc   start="\<\(m4_\)\=\(include\|sinclude\)("he=e-1 end=")" contains=@m4Top
syn region m4Command  matchgroup=m4Statement start="\<\(m4_\)\=\(syscmd\|esyscmd\|ifdef\|ifelse\|indir\|builtin\|shift\|errprint\|m4exit\|changecom\|changequote\|changeword\|m4wrap\|debugfile\|divert\|undivert\)("he=e-1 end=")" contains=@m4Top
syn region m4Command  matchgroup=m4builtin start="\<\(m4_\)\=\(len\|index\|regexp\|substr\|translit\|patsubst\|format\|incr\|decr\|eval\|maketemp\)("he=e-1 end=")" contains=@m4Top
syn keyword m4Statement divert undivert
syn region m4Command  matchgroup=m4Type      start="\<\(m4_\)\=\(undefine\|popdef\)("he=e-1 end=")" contains=@m4Top
syn region m4Function matchgroup=m4Type      start="\<[_A-Z]*("he=e-1 end=")" contains=@m4Top
syn region m4String   start="`" end="'" contained contains=@m4Top,@m4StringContents
syn cluster m4Top     contains=m4Comment,m4Constants,m4Special,m4Variable,m4String,m4Paren,m4Command,m4Statement,m4Function

if !exists("did_m4_syntax_inits")
  let did_m4_syntax_inits = 1
  " The default methods for highlighting.  Can be overridden later
  hi link m4Delimiter Delimiter
  hi link m4Comment   Comment
  hi link m4Function  Function
  hi link m4Keyword   Keyword
  hi link m4Special   Special
  hi link m4String    String
  hi link m4Statement Statement
  hi link m4Preproc   PreProc
  hi link m4Type      Type
  hi link m4Special   Special
  hi link m4Variable  Special
  hi link m4Constants Constant
  hi link m4Builtin   Statement
endif

let b:current_syntax = "m4"

if main_syntax == 'm4'
  unlet main_syntax
endif

" vim: ts=4