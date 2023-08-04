#
# ~/.bashrc
#

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias ls='ls --color=auto'
alias grep='grep --color=auto'
#alias neofetch='neofetch | sed s/jett/user/g'
#PS1='[\u@\h \W]\$ '
PS1='\[\e[1m\][\[\e[32m\]\u\[\e[37m\]@\[\e[35m\]\h\[\e[37m\] \W]\[\e[34m\] $\[\e[00m\] '
cat /home/jett/.cache/wal/sequences
