# top-k-words-of-txt-files-from-directory-path-recursively
top k words from all the text files from directory path recursively; usage: ./ssfi -t &lt;num of threads> &lt;file path> &lt;num of top K frequents words>

    -> traverse directory recursively and search for txt file 
    -> and search top k words from txt files found in worker threads 
    -> merge all top k words from all the txt file and print only top k words from these files
