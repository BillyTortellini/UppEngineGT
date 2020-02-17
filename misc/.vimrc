set nocp
set ruler
set number relativenumber
set expandtab
set softtabstop=0
set shiftwidth=4
set tabstop=4
set encoding=utf8
colo desert
filetype plugin on
syntax on
set backspace=indent,eol,start
set autoindent
set incsearch
imap <C-L> <Esc>
set formatoptions-=cro

nmap <F5> :silent !cls<CR>:wa<CR>:silent !O:\build\buildGameDLLWnd.bat<CR>
nmap <F6> :silent !O:\build\run.bat<CR>
nmap <F7> :silent !cls<CR>:wa<CR>:silent !O:\build\buildMainWnd.bat<CR>

so C:\Users\Martin\vimPlugins\plugins.vim

" Vim plugin options
nmap <C-P> :NERDTreeToggle<CR>

