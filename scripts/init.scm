(use-modules ((bow tree-sitter cursor)
	      #:prefix cursor:))

(define (walk-node cursor context)
  (let* ((node-type (cursor:current-node-type cursor)))
    (display (make-string (* 2 context) #\space))
    (display "- ")
    (display node-type)
    (newline)
    (when (cursor:goto-first-child cursor)
      (walk-node cursor (+ 1 context))

      (while (cursor:goto-next-sibling cursor)
	(walk-node cursor (+ 1 context)))
      
      (cursor:goto-parent cursor))))

(define-public (parse-tree cursor)
  (walk-node cursor 0))

(define-public (main)
  (find-file (string-append editor-base "sample/simple.js")))
   
