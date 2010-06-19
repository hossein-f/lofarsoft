c! modified for 2d arrays. can also wrap around and make it 1d but try this and see.
      SUBROUTINE mrqmin2d(y,sig,ndata,a,ia,ma,covar,alpha,nca,chisq,
     * alamda,fnum,cutoff)
      INTEGER ma,nca,ndata,ia(ma),MMAX,fnum
      REAL*8 alamda,chisq,a(ma),alpha(nca,nca),covar(nca,nca),
     *sig(ndata,ndata),y(ndata,ndata)
      PARAMETER (MMAX=20)
CU    USES covsrt,gaussj,mrqcof
      INTEGER j,k,l,mfit
      REAL*8 ochisq,atry(MMAX),beta(MMAX),da(MMAX),cutoff
      SAVE ochisq,atry,beta,da,mfit

      if(alamda.lt.0.d0)then
        mfit=0
        do 11 j=1,ma
          if (ia(j).ne.0) mfit=mfit+1
11      continue
        alamda=0.001
        call mrqcof2d(y,sig,ndata,a,ia,ma,alpha,beta,nca,chisq,
     /                fnum,cutoff)
        ochisq=chisq
        do 12 j=1,ma
          atry(j)=a(j)
12      continue
      endif
      do 14 j=1,mfit
        do 13 k=1,mfit
          covar(j,k)=alpha(j,k)
13      continue
        covar(j,j)=alpha(j,j)*(1.d0+alamda)
        da(j)=beta(j)
14    continue
      call gaussj(covar,mfit,nca,da,1,1)
      if(alamda.eq.0.d0)then
        call covsrt(covar,nca,ma,ia,mfit)
        return
      endif
      j=0
      do 15 l=1,ma
        if(ia(l).ne.0) then
          j=j+1
          atry(l)=a(l)+da(j)
        endif
15    continue
      call mrqcof2d(y,sig,ndata,atry,ia,ma,covar,da,nca,chisq,
     /              fnum,cutoff)
      if(chisq.lt.ochisq)then
        alamda=0.1*alamda
        ochisq=chisq
        do 17 j=1,mfit
          do 16 k=1,mfit
            alpha(j,k)=covar(j,k)
16        continue
          beta(j)=da(j)
17      continue
        do 18 l=1,ma
          a(l)=atry(l)
18      continue
      else
        alamda=10.d0*alamda
        chisq=ochisq
      endif
      return
      END
