c! to take .asrl file and write out associations in detail in .assf which is association file.
c! also to write another file to be used for plotting which is .pasf

        subroutine write_associatelist1(nsrcm,fluxm,ram,decm,bmajm,
     /         bminm,bpam,sstdm,savm,rstdm,ravm,nsrcs,fluxs,ras,decs,
     /         bmajs,bmins,bpas,sstds,savs,rstds,ravs,tol,scrat,
     /         master,second,dsum,scratch)
        implicit none
        integer nsrcm,nsrcs,nchar,magic
        real*8 fluxm(nsrcm),ram(nsrcm),decm(nsrcm),bmajm(nsrcm)
        real*8 bminm(nsrcm),bpam(nsrcm),rstdm(nsrcm),ravm(nsrcm)
        real*8 fluxs(nsrcs),ras(nsrcs),decs(nsrcs),bmajs(nsrcs)
        real*8 bmins(nsrcs),bpas(nsrcs),tol,dist,distold
        real*8 rstds(nsrcs),ravs(nsrcs)
        real*8 sstds(nsrcs),savs(nsrcs),sstdm(nsrcm),savm(nsrcm)
        character scrat*500,extn*10,fn*500,fn1*500,str1*1,str8*8
        character fmt1*2,fmt2*2,fmti*100,fn2*500,dir*500
        character master*(*),second*(*),dumc*20,scratch*500
        integer isrcm,isrcs,i,dumi,n_src_m2s,coden,nmul,j,mularr(nsrcm)
        integer nmult,dsum(5),idm
       
        extn='.asrl'
        fn=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        extn='.assf'
        fn1=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        extn='.pasf'
        fn2=scratch(1:nchar(scratch))//scrat(1:nchar(scrat))//
     /     extn(1:nchar(extn))
        write (*,*) 
        write (*,*) '  Writing out '//fn1(1:nchar(fn1))
        write (*,*) '  Writing out '//fn2(1:nchar(fn2))
        open(unit=21,file=fn,status='old')
        open(unit=22,file=fn1,status='unknown')
        open(unit=23,file=fn2,status='unknown')
        call sub_write_assoc_head(master,second,nsrcm,nsrcs,tol,22,dsum)
        read (21,*) magic                               ! signifies EOF
444     read (21,*) dumi
        if (dumi.eq.magic) then
         goto 333
        else
         backspace(21)
        end if
        read (21,*) isrcm,n_src_m2s
        call get_fmt_int(isrcm,fmt1)
        call get_fmt_int(n_src_m2s,fmt2)
        if (isrcm.eq.0) then 
         call write_srclist_singlekothi(n_src_m2s,22)
        else
         fmti="(a14,"//fmt1//",a8,"//fmt2//",a13)"
         write (22,fmti) 'Master source ',isrcm,' ;  Has ',
     /                n_src_m2s,' associations'
         distold=9.d99
         idm=99999
         do i=1,n_src_m2s
          read (21,*) isrcs,str1 
          if (str1.eq.'s') str8='singly  '
          if (str1.eq.'m') str8='multiply'
          call get_fmt_int(isrcs,fmt1)
          fmti="(a25,"//fmt1//",a11,a8,a11)"
          write (22,fmti) 'Associated sub source is ',isrcs,
     /       ', which is ',str8,' associated'
          call justdist(ram(isrcm),ras(isrcs),decm(isrcm),decs(isrcs),
     /         dist)
          if (dist.lt.distold) then
           idm=isrcs
           distold=dist
          end if
         end do
         do i=1,n_src_m2s
          backspace(21)
         end do
         coden=2
         do i=1,n_src_m2s
          read (21,*) isrcs,str1 
          if (i.ne.1) coden=1
          call sub_write_assoclist1(fluxm(isrcm),ram(isrcm),decm(isrcm)
     /         ,bmajm(isrcm),bminm(isrcm),bpam(isrcm),fluxs(isrcs),
     /         ras(isrcs),decs(isrcs),bmajs(isrcs),bmins(isrcs),
     /         bpas(isrcs),sstds(isrcs),rstds(isrcs),22,coden,isrcm,
     /         isrcs,'Master ','Second ')
          if (str1.eq.'s') then
           write (23,*) isrcm,fluxm(isrcm),ram(isrcm),decm(isrcm) ! avoiding fmt
     /         ,bmajm(isrcm),bminm(isrcm),bpam(isrcm),sstds(idm),
     /         rstds(idm)
           write (23,*) idm,fluxs(idm),ras(idm),decs(idm)
     /         ,bmajs(idm),bmins(idm),bpas(idm),distold
          end if
         end do
        end if
        write (22,*)
        goto 444
333     continue
        close(21)
c!                                              now for slutkothis
        call system('rm -f a b')
        call system('grep m '//fn(1:nchar(fn))//' > a')
        call system('sort -u -g a > b')
        dumc='b'
        dir="./"
        call getline(dumc,dir,nmult)
        open(unit=21,file='b',status='old')
        do i=1,nmult
         read (21,*) isrcs,str1,nmul
         call sub_write_assoc_getmulsrcid(21,nmul,mularr,nsrcm)
         call get_fmt_int(isrcs,fmt1)
         fmti="(a13,1x,"//fmt1//",1x,a22)"
         write (22,fmti) 'Second source',isrcs,'is multiply associated'
         coden=2
         distold=9.d99
         idm=99999
         do j=1,nmul
          if (j.ne.1) coden=1
          call sub_write_assoclist1(fluxs(isrcs),ras(isrcs),decs(isrcs)
     /      ,bmajs(isrcs),bmins(isrcs),bpas(isrcs),fluxm(mularr(j)),
     /      ram(mularr(j)),decm(mularr(j)),bmajm(mularr(j)),
     /      bminm(mularr(j)),bpam(mularr(j)),sstds(isrcs),rstds(isrcs),
     /      22,coden,isrcs,mularr(j),'Second ','Master ')
          call justdist(ram(mularr(j)),ras(isrcs),decm(mularr(j)),
     /         decs(isrcs),dist)
          if (dist.lt.distold) then
           idm=mularr(j)
           distold=dist
          end if
         end do
         write (23,*) idm,fluxm(idm),ram(idm),decm(idm)
     /       ,bmajm(idm),bminm(idm),bpam(idm),sstds(isrcs),rstds(isrcs)
         write (23,*) isrcs,fluxs(isrcs),ras(isrcs),decs(isrcs)
     /       ,bmajs(isrcs),bmins(isrcs),bpas(isrcs),distold
         write (22,*)
        end do
        close(21)
        close(22)
        close(23)
        
        return
        end
c!
c! ----------------------------   SUBROUTINES   --------------------------------
c!
        subroutine sub_write_assoclist1(flux1,ra1,dec1,bmaj1,bmin1,
     /        bpa1,flux2,ra2,dec2,bmaj2,bmin2,bpa2,sstd,rstd,nn,coden,
     /        idm,ids,f1,f2)
        implicit none
        real*8 flux1,ra1,dec1,bmaj1,bmin1,bpa1,ss1,sa1
        real*8 flux2,ra2,dec2,bmaj2,bmin2,bpa2,dist,ss2,sa2,rstd,sstd
        integer hh1,hh2,dd1,dd2,mm1,mm2,ma1,ma2,nn,coden,idm,ids,nchar
        character s1,s2,fmt1*2,fmta*500,f1*7,f2*7,rstd1*18,rstd2*18
        character fmta1*500,fmta2*500,sstd1*18,sstd2*18

        call justdist(ra1,ra2,dec1,dec2,dist)
        call convertra(ra1,hh1,mm1,ss1)
        call convertra(ra2,hh2,mm2,ss2)
        call convertdec(dec1,s1,dd1,ma1,sa1)
        call convertdec(dec2,s2,dd2,ma2,sa2)
        call get_fmt_int(max(idm,ids),fmt1)
        fmta="(a7,"//fmt1//",a3,0Pf10.4,a8,i2,1x,i2,1x,0Pf5.2,1x,"//
     /    "a1,i2,1x,i2,1x,0Pf5.2,1x,a2,0Pf7.3,a3,0Pf7.3,a3,0Pf7.3"
        if (f1.eq.'Second ') then
         fmta1=fmta(1:nchar(fmta))//",a18,a18)"
         fmta2=fmta(1:nchar(fmta))//",a1,a9,0Pf7.3,a1)"
         write (sstd1,'(a8,0Pf10.4)') '; rms : ',sstd
         write (rstd1,'(a8,0Pf10.4)') '; rms : ',rstd
         sstd2=' '
         rstd2=' '
        end if
        if (f1.eq.'Master ') then
         fmta1=fmta(1:nchar(fmta))//",a1)"
         fmta2=fmta(1:nchar(fmta))//",a18,a18,a9,0Pf7.3,a1)"
         write (sstd2,'(a8,0Pf10.4)') '; rms : ',sstd
         write (rstd2,'(a8,0Pf10.4)') '; rms : ',rstd
         sstd1=' '
         rstd1=' '
        end if
        if (coden.eq.2) 
     /  write (nn,fmta1) f1,idm,' : ',flux1,' Jy; At ',hh1,mm1,
     /        ss1,s1,dd1,ma1,sa1,'; ',bmaj1,' X ',bmin1,' X ',bpa1,
     /        sstd1,rstd1
        write (nn,fmta2) f2,ids,' : ',flux2,' Jy; At ',hh2,mm2,
     /        ss2,s2,dd2,ma2,sa2,'; ',bmaj2,' X ',bmin2,' X ',bpa2
     /        ,sstd2,rstd2,'; Dist = ',dist,'"'
        
        return
        end
c!
c!      ----------
c!
        subroutine write_srclist_singlekothi(ids,nn)
        implicit none
        integer ids,nn
        character fmt1*2,fmta*100

        call get_fmt_int(ids,fmt1)
        fmta="(a14,"//fmt1//",a23,a11)"
        write (nn,fmta) 'Second source ',ids,' has no association in '
     /        ,'master list'
        
        return
        end
c!
c!      ----------
c!
        subroutine sub_write_assoc_head(master,second,nm,ns,tol,nn,dsum)
        implicit none
        character master*(*),second*(*)
        integer nm,ns,nn,nchar,dsum(5)
        real*8 tol

        write (nn,*)
        write (nn,'(10x,a)') 'Association of Sourcelist file'
        write (nn,'(10x,a)') '=============================='
        write (nn,*)
        write (nn,*) 'Master source list : ',master(1:nchar(master)),
     /        ' (',nm,' srcs)'
        write (nn,*) 'Second source list : ',second(1:nchar(second)),
     /        ' (',ns,' srcs)'
        write (nn,*) 'Tolerance for association : ',tol,' arcsec'
        write (nn,*) 
        write (nn,*) 'Found ',dsum(1),' associations with ',
     /       master(1:nchar(master))
        write (nn,*)  'Sources in ',master(1:nchar(master)),
     /       ' with no associations is ',dsum(2)
        write (nn,*)  'Sources in ',second(1:nchar(second)),
     /       ' with no associations is ',dsum(3)
        write (nn,*)  'Sources in ',master(1:nchar(master)),
     /       ' with multiple associations is ',dsum(4)
        write (nn,*)  'Sources in ',second(1:nchar(second)),
     /       ' with multiple associations is ',dsum(5)
        write (nn,*) 

        return
        end
c!
c!      ----------
c!
        subroutine sub_write_assoc_getmulsrcid(nn,nmul,mularr,x)
        implicit none
        integer nn,x,mularr(x),nmul,vec(nmul),dumi,i
        character dumc

        backspace(nn)
        read (nn,*) dumi,dumc,dumi,vec
        do i=1,nmul
         mularr(i)=vec(i)
        end do

        return
        end
c!
c!      ----------
c!
