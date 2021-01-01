(use-modules ((bow tree-sitter cursor)
	      #:prefix cursor:))

(define (walk-node cursor parents)
  (let* ((node-type (cursor:current-node-type cursor)))
    (display node-type)
    (newline)
    (when (cursor:goto-first-child cursor)
      (walk-node cursor parents)
      (cursor:goto-parent cursor))))

(define-public (parse-tree cursor)
  (walk-node cursor (list)))

(define-public (main)
  (find-file (string-append editor-base "sample/simple.js")))
   
