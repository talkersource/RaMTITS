/* Thanks to Cygnus for sending me this. What this does is determine which -
 - setting to use to set a socket to non-blocking on different systems.    */
#if defined(FNONBLOCK)		/* SYSV,AIX,SOLARIS,IRIX,HP-UX */
  #define NBLOCK_CMD FNONBLOCK
#else
  #if defined(O_NDELAY)		/* BSD,LINUX,SOLARIS,IRIX */
    #define NBLOCK_CMD O_NDELAY
  #else
    #if defined(FNDELAY)	/* BSD,LINUX,SOLARIS,IRIX */
      #define NBLOCK_CMD FNDELAY
    #else
      #if define(FNBIO)		/* SYSV */
        #define NBLOCK_CMD FNBIO
      #else
        #if defined(FNONBIO)	/* ?? */
          #define NBLOCK_CMD FNONBIO
        #else
          #if defined(FNONBLK)	/* IRIX */
            #define NBLOCK_CMD FNONBLK
          #else
            #define NBLOCK_CMD 0
          #endif
        #endif
      #endif
    #endif
  #endif
#endif
