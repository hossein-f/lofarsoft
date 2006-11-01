c! multiply image in f1 with modulation image in f2 and write to image f3

        subroutine modulate(f1,f2,f3,scratch)
        implicit none
        character f1*(*),f2*(*),f3*(*),extn*10,scratch*500
        integer i,j,n,m

        extn='.img'
        call readarraysize(f1,extn,n,m)
        call readarraysize(f2,extn,i,j)
        if (n.ne.i.or.m.ne.j) then
         write (*,*) ' ###  Image and modulation arent of same size ###'
         write (*,*) ' ###          Exiting modulation program      ###'
         goto 333
        end if
        call sub_modulate(f1,f2,f3,n,m,scratch)
333     continue

        return
        end
c!
c!
        subroutine sub_modulate(f1,f2,f3,n,m,scratch)
        implicit none
        character f1*(*),f2*(*),f3*(*),lab*500,strdev*5,extn*10
        character scratch*500
        integer nchar,i,j,n,m
        real*8 fimage(n,m),image1(n,m),image2(n,m)
        real*8 a(3),sigmaJy

        call readarray_bin(n,m,image1,n,m,f1,extn)
        call readarray_bin(n,m,image2,n,m,f2,extn)
        call imstat(image1,n,m,n,m,a,'qn')
        sigmaJy=a(3)
        do 100 i=1,n
         do 110 j=1,m
          fimage(i,j)=image1(i,j)*(1.d0+image2(i,j)/sigmaJy)
110      continue
100     continue
        
        call writearray_bin(fimage,n,m,n,m,f3,'mv')

        write (*,'(a34,$)') '   Image of final noise ... <RET> '
        read (*,*)
        lab='Final noise image'
        strdev='/xs'
        call grey(fimage,n,m,n,m,strdev,lab,0,1,'hn',1,scratch)

        return
        end


