" Vim syntax file
" Language:	Diff (context or unified)
" Maintainer:	Bram Moolenaar <Bram@vim.org>
" Last change:	1999 Jun 28

" Remove any old syntax stuff hanging around
syn clear

syn match diffOnly	"^Only in.*"

syn match diffRemoved	"^-.*"
syn match diffRemoved	"^<.*"
syn match diffAdded	"^+.*"
syn match diffAdded	"^>.*"
syn match diffChanged	"^! .*"

syn match diffLine	"^@.*"
syn match diffLine	"^\<\d\+\>.*"
syn match diffLine	"^\*\*\*\*.*"

"Some versions of diff have lines like "#c#" and "#d#" (where # is a number)
syn match diffLine	"^\d\+\(,\d\+\)\=[cda]\d\+\>.*"

syn match diffFile	"^diff.*"
syn match diffFile	"^+++ .*"
syn match diffFile	"^Index: .*$"
syn match diffOldFile	"^\*\*\* .*"
syn match diffNewFile	"^--- .*"

syn match diffComment	"^#.*"

if !exists("did_diff_syntax_inits")
  let did_diff_syntax_inits = 1
  hi link diffOldFile	diffFile
  hi link diffNewFile	diffFile
  hi link diffFile	Type
  hi link diffOnly	Constant
  hi link diffRemoved	Special
  hi link diffChanged	PreProc
  hi link diffAdded	Identifier
  hi link diffLine	Statement
  hi link diffComment	Comment
endif

let b:current_syntax = "diff"

" vim: ts=8