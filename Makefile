nothing:

# NEWPROBELM

merge-pdfs: pA/attachments/problems.pdf

pA/attachments/problems.pdf: p*/statement/index.pdf
	bash merge_pdfs.sh
