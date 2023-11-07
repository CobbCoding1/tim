" Vim syntax file
" Language: TASM (Titanium assembly) 

" Usage Instructions
" Put this file in .vim/syntax/titasm.vim or in
" your $VIMSOURCE, which may be found in /usr/share/nvim/runtime/syntax
" and in your .vimrc file add the following line:
" autocmd BufRead,BufNewFile *.tasm,*.tash set filetype=tasm
" or for neovim, add this command to your init.lua
" vim.cmd[[
"   augroup FileTypeSettings
"     autocmd!
"     autocmd BufRead,BufNewFile *.tasm,*.tash set filetype=tasm
"   augroup END
" ]]


if exists("b:current_syntax")
  finish
endif

syntax keyword tasmTodos TODO NOTE

" Language keywords
syntax keyword tasmKeywords nop push pop dup indup
syntax keyword tasmKeywords add sub mul div mod
syntax keyword tasmKeywords mul_f div_f mod_f
syntax keyword tasmKeywords add_f sub_f
syntax keyword tasmKeywords jmp zjmp nzjmp halt swap inswap
syntax keyword tasmKeywords cmpe cmpl cmpg cmple cmpge cmpne 
syntax keyword tasmKeywords ret call native
syntax keyword tasmKeywords itof ftoi push_ptr push_str get_str mov
syntax keyword tasmKeywords mov_str ref deref pop_str
syntax keyword tasmKeywords swap_str inswap_str index
syntax keyword tasmKeywords entrypoint top print
syntax match tasmRegister /r\%(3[0-1]\|[0-2]\?\d\)/

" Comments
syntax region tasmCommentLine start=";" end="$"   contains=tasmTodos
syntax region tasmDirective start="@" end=" "

syntax match tasmLabel		"[a-z_][a-z0-9_]*:"he=e-1

" Numbers
syntax match tasmDecInt display "\<[0-9][0-9_]*"
"syntax match tasmHexInt display "\<0[xX][0-9a-fA-F][0-9_a-fA-F]*"
syntax match tasmFloat  display "\<[0-9][0-9_]*\%(\.[0-9][0-9_]*\)"

" Strings
syntax region tasmString start=/\v"/ skip=/\v\\./ end=/\v"/
syntax region tasmString start=/\v'/ skip=/\v\\./ end=/\v'/

" Set highlights
highlight default link tasmTodos Todo
highlight default link tasmKeywords Identifier
highlight default link tasmCommentLine Comment
highlight default link tasmDirective PreProc
highlight default link tasmLoopKeywords PreProc
highlight default link tasmDecInt Number
highlight default link tasmHexInt Number
highlight default link tasmFloat Float
highlight default link tasmString String
highlight default link tasmLabel Label
highlight default link tasmRegister StorageClass 

let b:current_syntax = "titasm"

