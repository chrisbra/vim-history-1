" Vim syntax file
" Language:	Pine (email program) run commands
" Maintainer:	David Pascoe <David.Pascoe@jtec.com.au>
" Last Change:	Thu Apr 26 10:39:41 WST 2001, updated for pine 4.33

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

if version >= 600
  setlocal iskeyword=@,48-57,_,128-167,224-235,-,
else
  set iskeyword=@,48-57,_,128-167,224-235,-,
endif

syn keyword pineConfig addrbook-sort-rule
syn keyword pineConfig address-book
syn keyword pineConfig addressbook-formats
syn keyword pineConfig alt-addresses
syn keyword pineConfig bugs-additional-data
syn keyword pineConfig bugs-address
syn keyword pineConfig bugs-fullname
syn keyword pineConfig character-set
syn keyword pineConfig color-style
syn keyword pineConfig compose-mime
syn keyword pineConfig composer-wrap-column
syn keyword pineConfig cursor-style
syn keyword pineConfig customized-hdrs
syn keyword pineConfig default-composer-hdrs
syn keyword pineConfig default-fcc
syn keyword pineConfig default-saved-msg-folder
syn keyword pineConfig disable-these-drivers
syn keyword pineConfig display-filters
syn keyword pineConfig download-command
syn keyword pineConfig download-command-prefix
syn keyword pineConfig editor
syn keyword pineConfig elm-style-save
syn keyword pineConfig empty-header-message
syn keyword pineConfig fcc-name-rule
syn keyword pineConfig feature-level
syn keyword pineConfig feature-list
syn keyword pineConfig file-directory
syn keyword pineConfig folder-collections
syn keyword pineConfig folder-extension
syn keyword pineConfig folder-sort-rule
syn keyword pineConfig font-name
syn keyword pineConfig font-size
syn keyword pineConfig font-style
syn keyword pineConfig forced-abook-entry
syn keyword pineConfig form-letter-folder
syn keyword pineConfig global-address-book
syn keyword pineConfig goto-default-rule
syn keyword pineConfig header-in-reply
syn keyword pineConfig image-viewer
syn keyword pineConfig inbox-path
syn keyword pineConfig incoming-archive-folders
syn keyword pineConfig incoming-folders
syn keyword pineConfig incoming-startup-rule
syn keyword pineConfig index-answered-background-color
syn keyword pineConfig index-answered-foreground-color
syn keyword pineConfig index-deleted-background-color
syn keyword pineConfig index-deleted-foreground-color
syn keyword pineConfig index-format
syn keyword pineConfig index-important-background-color
syn keyword pineConfig index-important-foreground-color
syn keyword pineConfig index-new-background-color
syn keyword pineConfig index-new-foreground-color
syn keyword pineConfig index-recent-background-color
syn keyword pineConfig index-recent-foreground-color
syn keyword pineConfig index-to-me-background-color
syn keyword pineConfig index-to-me-foreground-color
syn keyword pineConfig index-unseen-background-color
syn keyword pineConfig index-unseen-foreground-color
syn keyword pineConfig initial-keystroke-list
syn keyword pineConfig kblock-passwd-count
syn keyword pineConfig keylabel-background-color
syn keyword pineConfig keylabel-foreground-color
syn keyword pineConfig keyname-background-color
syn keyword pineConfig keyname-foreground-color
syn keyword pineConfig last-time-prune-questioned
syn keyword pineConfig last-version-used
syn keyword pineConfig ldap-servers
syn keyword pineConfig local-address
syn keyword pineConfig local-fullname
syn keyword pineConfig mail-check-interval
syn keyword pineConfig mail-directory
syn keyword pineConfig mailcap-search-path
syn keyword pineConfig mimetype-search-path
syn keyword pineConfig new-version-threshold
syn keyword pineConfig news-active-file-path
syn keyword pineConfig news-collections
syn keyword pineConfig news-spool-directory
syn keyword pineConfig newsrc-path
syn keyword pineConfig nntp-new-group-time
syn keyword pineConfig nntp-server
syn keyword pineConfig normal-background-color
syn keyword pineConfig normal-foreground-color
syn keyword pineConfig old-style-reply
syn keyword pineConfig operating-dir
syn keyword pineConfig patterns
syn keyword pineConfig personal-name
syn keyword pineConfig personal-print-category
syn keyword pineConfig personal-print-command
syn keyword pineConfig postponed-folder
syn keyword pineConfig print-font-name
syn keyword pineConfig print-font-size
syn keyword pineConfig print-font-style
syn keyword pineConfig printer
syn keyword pineConfig prompt-background-color
syn keyword pineConfig prompt-foreground-color
syn keyword pineConfig pruned-folders
syn keyword pineConfig quote1-background-color
syn keyword pineConfig quote1-foreground-color
syn keyword pineConfig quote2-background-color
syn keyword pineConfig quote2-foreground-color
syn keyword pineConfig quote3-background-color
syn keyword pineConfig quote3-foreground-color
syn keyword pineConfig read-message-folder
syn keyword pineConfig remote-abook-history
syn keyword pineConfig remote-abook-metafile
syn keyword pineConfig remote-abook-validity
syn keyword pineConfig reply-indent-string
syn keyword pineConfig reply-leadin
syn keyword pineConfig reverse-background-color
syn keyword pineConfig reverse-foreground-color
syn keyword pineConfig rsh-command
syn keyword pineConfig rsh-open-timeout
syn keyword pineConfig rsh-path
syn keyword pineConfig save-by-sender
syn keyword pineConfig saved-msg-name-rule
syn keyword pineConfig scroll-margin
syn keyword pineConfig selectable-item-background-color
syn keyword pineConfig selectable-item-foreground-color
syn keyword pineConfig sending-filters
syn keyword pineConfig sendmail-path
syn keyword pineConfig show-all-characters
syn keyword pineConfig signature-file
syn keyword pineConfig smtp-server
syn keyword pineConfig sort-key
syn keyword pineConfig speller
syn keyword pineConfig ssh-command
syn keyword pineConfig ssh-open-timeout
syn keyword pineConfig ssh-path
syn keyword pineConfig standard-printer
syn keyword pineConfig status-background-color
syn keyword pineConfig status-foreground-color
syn keyword pineConfig status-message-delay
syn keyword pineConfig suggest-address
syn keyword pineConfig suggest-fullname
syn keyword pineConfig tcp-open-timeout
syn keyword pineConfig title-background-color
syn keyword pineConfig title-foreground-color
syn keyword pineConfig upload-command
syn keyword pineConfig upload-command-prefix
syn keyword pineConfig url-viewers
syn keyword pineConfig use-only-domain-name
syn keyword pineConfig user-domain
syn keyword pineConfig user-id
syn keyword pineConfig user-id
syn keyword pineConfig user-input-timeout
syn keyword pineConfig viewer-hdr-colors
syn keyword pineConfig viewer-hdrs
syn keyword pineConfig viewer-overlap
syn keyword pineConfig window-position

syn keyword pineOption add-ldap-result-to-addrbook
syn keyword pineOption allow-changing-from
syn keyword pineOption allow-talk
syn keyword pineOption assume-slow-link
syn keyword pineOption auto-move-read-msgs
syn keyword pineOption auto-open-next-unread
syn keyword pineOption auto-unzoom-after-apply
syn keyword pineOption auto-zoom-after-select
syn keyword pineOption combined-addrbook-display
syn keyword pineOption combined-folder-display
syn keyword pineOption combined-subdirectory-display
syn keyword pineOption compose-cut-from-cursor
syn keyword pineOption compose-maps-delete-key-to-ctrl-d
syn keyword pineOption compose-posts-in-background
syn keyword pineOption compose-rejects-unqualified-addrs
syn keyword pineOption compose-send-offers-first-filter
syn keyword pineOption compose-sets-newsgroup-without-confirm
syn keyword pineOption confirm-role-even-for-default
syn keyword pineOption delete-skips-deleted
syn keyword pineOption disable-busy-alarm
syn keyword pineOption disable-config-cmd
syn keyword pineOption disable-keyboard-lock-cmd
syn keyword pineOption disable-keymenu
syn keyword pineOption disable-password-cmd
syn keyword pineOption disable-pipes-in-sigs
syn keyword pineOption disable-pipes-in-templates
syn keyword pineOption disable-roles-setup-cmd
syn keyword pineOption disable-roles-sig-edit
syn keyword pineOption disable-roles-template-edit
syn keyword pineOption disable-signature-edit-cmd
syn keyword pineOption disable-take-last-comma-first
syn keyword pineOption enable-8bit-esmtp-negotiation
syn keyword pineOption enable-8bit-nntp-posting
syn keyword pineOption enable-aggregate-command-set
syn keyword pineOption enable-alternate-editor-cmd
syn keyword pineOption enable-alternate-editor-implicitly
syn keyword pineOption enable-arrow-navigation
syn keyword pineOption enable-arrow-navigation-relaxed
syn keyword pineOption enable-background-sending
syn keyword pineOption enable-bounce-cmd
syn keyword pineOption enable-cruise-mode
syn keyword pineOption enable-cruise-mode-delete
syn keyword pineOption enable-delivery-status-notification
syn keyword pineOption enable-dot-files
syn keyword pineOption enable-dot-folders
syn keyword pineOption enable-exit-via-lessthan-command
syn keyword pineOption enable-fast-recent-test
syn keyword pineOption enable-flag-cmd
syn keyword pineOption enable-flag-screen-implicitly
syn keyword pineOption enable-full-header-cmd
syn keyword pineOption enable-goto-in-file-browser
syn keyword pineOption enable-incoming-folders
syn keyword pineOption enable-jump-shortcut
syn keyword pineOption enable-lame-list-mode
syn keyword pineOption enable-mail-check-cue
syn keyword pineOption enable-mailcap-param-substitution
syn keyword pineOption enable-mouse-in-xterm
syn keyword pineOption enable-msg-view-addresses
syn keyword pineOption enable-msg-view-attachments
syn keyword pineOption enable-msg-view-forced-arrows
syn keyword pineOption enable-msg-view-urls
syn keyword pineOption enable-msg-view-web-hostnames
syn keyword pineOption enable-newmail-in-xterm-icon
syn keyword pineOption enable-partial-match-lists
syn keyword pineOption enable-print-via-y-command
syn keyword pineOption enable-reply-indent-string-editing
syn keyword pineOption enable-rules-under-take
syn keyword pineOption enable-search-and-replace
syn keyword pineOption enable-sigdashes
syn keyword pineOption enable-suspend
syn keyword pineOption enable-tab-completion
syn keyword pineOption enable-tray-icon
syn keyword pineOption enable-unix-pipe-cmd
syn keyword pineOption enable-verbose-smtp-posting
syn keyword pineOption expanded-view-of-addressbooks
syn keyword pineOption expanded-view-of-distribution-lists
syn keyword pineOption expanded-view-of-folders
syn keyword pineOption expunge-without-confirm
syn keyword pineOption expunge-without-confirm-everywhere
syn keyword pineOption fcc-on-bounce
syn keyword pineOption fcc-only-without-confirm
syn keyword pineOption fcc-without-attachments
syn keyword pineOption include-attachments-in-reply
syn keyword pineOption include-header-in-reply
syn keyword pineOption include-text-in-reply
syn keyword pineOption ldap-result-to-addrbook-add
syn keyword pineOption news-approximates-new-status
syn keyword pineOption news-deletes-across-groups
syn keyword pineOption news-offers-catchup-on-close
syn keyword pineOption news-post-without-validation
syn keyword pineOption news-read-in-newsrc-order
syn keyword pineOption no-print-index-enabled
syn keyword pineOption no-signature-at-bottom
syn keyword pineOption no-old-growth
syn keyword pineOption no-include-header-in-reply
syn keyword pineOption old-growth
syn keyword pineOption pass-control-characters-as-is
syn keyword pineOption preserve-start-stop-characters
syn keyword pineOption print-formfeed-between-messages
syn keyword pineOption print-includes-from-line
syn keyword pineOption print-index-enabled
syn keyword pineOption print-offers-custom-cmd-prompt
syn keyword pineOption quell-berkeley-format-timezone
syn keyword pineOption quell-dead-letter-on-cancel
syn keyword pineOption quell-empty-directories
syn keyword pineOption quell-folder-internal-msg
syn keyword pineOption quell-imap-envelope-update
syn keyword pineOption quell-lock-failure-warnings
syn keyword pineOption quell-news-envelope-update
syn keyword pineOption quell-partial-fetching
syn keyword pineOption quell-status-message-beeping
syn keyword pineOption quell-user-lookup-in-passwd-file
syn keyword pineOption quit-without-confirm
syn keyword pineOption reply-always-uses-reply-to
syn keyword pineOption save-aggregates-copy-sequence
syn keyword pineOption save-will-advance
syn keyword pineOption save-will-not-delete
syn keyword pineOption save-will-quote-leading-froms
syn keyword pineOption select-without-confirm
syn keyword pineOption selectable-item-nobold
syn keyword pineOption separate-folder-and-directory-entries
syn keyword pineOption show-cursor
syn keyword pineOption show-plain-text-internally
syn keyword pineOption show-selected-in-boldface
syn keyword pineOption signature-at-bottom
syn keyword pineOption single-column-folder-list
syn keyword pineOption strip-from-sigdashes-on-reply
syn keyword pineOption tab-visits-next-new-message-only
syn keyword pineOption termdef-takes-precedence
syn keyword pineOption try-alternative-authentication-driver-first
syn keyword pineOption use-current-dir
syn keyword pineOption use-function-keys
syn keyword pineOption use-old-unix-format-write
syn keyword pineOption use-sender-not-x-sender
syn keyword pineOption use-subshell-for-suspend
syn keyword pineOption vertical-folder-list

syn match  pineComment  "^#.*$"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_pine_syn_inits")
  if version < 508
    let did_pine_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink pineConfig	Type
  HiLink pineComment	Comment
  HiLink pineOption	Macro
  delcommand HiLink
endif

let b:current_syntax = "pine"

" vim: ts=8
