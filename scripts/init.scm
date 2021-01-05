(use-modules ((srfi srfi-1))
	     ((ice-9 regex))
	     ((bow tree-sitter cursor)
	      #:prefix cursor:))

(define keywords-regexp
  (make-regexp "^(function|return|const|var|let|else|if|while|do|or|not|new)$"))

(define (node-type-keyword? type)
  (regexp-match?
   (regexp-exec keywords-regexp type)))

(define (walk-node cursor context)
  (let* ((node (cursor:current-node->alist cursor))
	 (node-type  (first node))
	 (node-start (second node))
	 (node-end   (third node)))
    
    ;; (display (make-string (* 2 context) #\space))
    ;; (display "* ")
    ;; (display node-type)

    (cond ((string= node-type "comment")
	   (buffer-apply-tag "comment" node-start node-end))

	  ((string= node-type "string")
	   (buffer-apply-tag "string" node-start node-end))

	  ((string= node-type "number")
	   (buffer-apply-tag "number" node-start node-end))

	  ((node-type-keyword? node-type)
	   (buffer-apply-tag "keyword" node-start node-end))
	  )

    (when (cursor:goto-first-child cursor)
      (walk-node cursor (+ 1 context))

      (while (cursor:goto-next-sibling cursor)
	(walk-node cursor (+ 1 context)))
      
      (cursor:goto-parent cursor))))

(define-public (parse-tree cursor)
  (walk-node cursor 0))

(define-public (main)
  (find-file (string-append editor-base "sample/react.development.js")))
   
