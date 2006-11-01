c! to n-bit an image

        subroutine nbit(f1,scratch)
        implicit none
        character f1*(*),extn*10,scratch*500
        integer n,m

        extn='.img'
        call readarraysize(f1,extn,n,m)
        call sub_nbit(f1,n,m,scratch)
        
        return
        end

        subroutine sub_nbit(f1,n,m,scratch)
        implicit none
        character f1*(*),extn*10,strdev*5,lab*500,ch1*1,f2*500
        character scratch*500
        integer n,m,lbit,nlev,nbit,i,j,ilev,nchar
        real*8 image(n,m),bowndry(256),bitim(n,m)

        extn='.img'
        call readarray_bin(n,m,image,n,m,f1,extn)

        strdev='/xs'
        lab=' '
        call grey(image,n,m,n,m,strdev,lab,1,0,'hg',0,scratch)

333     continue
        write (*,*) '  Will n-bit the image with integer values'
        write (*,'(a33,$)') '   Enter n for n-bit-ing image : '
        read (*,*) nbit
        if (nbit.lt.1.or.nbit.gt.8) goto 333
        write(*,'(a21,$)') '  Lowest value (0) : '
        read (*,'(a1)') ch1
        if (ichar(ch1).eq.32) then
         lbit=0
        else
         read (ch1,*) lbit
        end if

        nlev=2**nbit
        write (*,*) '  Enter the ',nlev-1,' values : '
        call system('rm -f a b')
        call getininp 
        call readininp(nlev-1)
        open(unit=21,file='b',status='old')
        do 100 i=1,nlev-1
         read (21,*) bowndry(i)
100     continue
        close(21)

        do i=1,n
         do j=1,m
          do 300 ilev=1,nlev-1
           if (image(i,j).ge.bowndry(ilev)) then
            bitim(i,j)=lbit*1.d0+ilev
           end if
300       continue
          if (image(i,j).lt.bowndry(1)) bitim(i,j)=lbit*1.d0
         end do
        end do

        f2=f1(1:nchar(f1))//'.nbit'
        call writearray_bin(bitim,n,m,n,m,f2,'mv')

        return
        end

