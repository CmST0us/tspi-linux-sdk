
#include <time.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include "../glprocess.h"
#include <libdrm/drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <unistd.h>
typedef void * (*__doByGpuCreate)();
typedef int (*__doByGpuInit)(void *p,int screenW,int screenH,int priority);
typedef int (*__doByGpuComposition)(void *p,layer_list_t *layerinfo) ;
typedef void * (*__doByGpucreateFence)(void *p);
typedef int (*__doByGpuwaitFence)(void *p,void *fence);
typedef int (*__doByGpuDestroy)(void *p);


#define RK_XXX_PATH         "/usr/lib/libgl_process.so"

void *buf_alloc(int *fd,int width,int height)
{
    static const char* card = "/dev/dri/card0";
    int flag = O_RDWR;
    int ret;
    void *map = NULL;
    static int drm_fd = -1;

    void *vir_addr = NULL;
    struct drm_prime_handle fd_args;
    struct drm_mode_map_dumb mmap_arg;
    struct drm_mode_destroy_dumb destory_arg;

    struct drm_mode_create_dumb alloc_arg;

    if(drm_fd < 0)
    {
        drm_fd = open(card, flag);
    }    
    if(drm_fd < 0)
    {
        printf("failed to open %s\n", card);
        return NULL;
    }

    memset(&alloc_arg, 0, sizeof(alloc_arg));
    alloc_arg.bpp = 32; 
    alloc_arg.width = width;
    alloc_arg.height = height *1.5 ;

    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &alloc_arg);
    if (ret) {
        printf("failed to create dumb buffer: %s\n", strerror(errno));
        return NULL;
    }

    memset(&fd_args, 0, sizeof(fd_args));
	fd_args.fd = -1;
	fd_args.handle = alloc_arg.handle;;
	fd_args.flags = 0;
	ret = drmIoctl(drm_fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &fd_args);
	if (ret)
	{
	    printf("rk-debug handle_to_fd failed ret=%d,err=%s, handle=%x \n",ret,strerror(errno),fd_args.handle);
		return NULL;
    }
    //printf("Dump fd = %d \n",fd_args.fd);
    *fd = fd_args.fd;

  //handle to Virtual address
    memset(&mmap_arg, 0, sizeof(mmap_arg));
    mmap_arg.handle = alloc_arg.handle;

    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &mmap_arg);
    if (ret) {
        printf("failed to create map dumb: %s\n", strerror(errno));
        vir_addr = NULL;
        goto destory_dumb;
    }
    vir_addr = map = mmap64(0, alloc_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, mmap_arg.offset);
    if (map == MAP_FAILED) {
        printf("failed to mmap buffer: %s\n", strerror(errno));
        vir_addr = NULL;
        goto destory_dumb;
    }
   // printf("alloc map=%x \n",map);
    return vir_addr;
destory_dumb:
  memset(&destory_arg, 0, sizeof(destory_arg));
  destory_arg.handle = alloc_arg.handle;
  int fdd = *fd ;
  ret = drmIoctl(fdd, DRM_IOCTL_MODE_DESTROY_DUMB, &destory_arg);
  if (ret)
    printf("failed to destory dumb %d\n", ret);
  return vir_addr;
}

#define   MaxTex 160

static void *render_thread(void *)
{
    int outwidth = 1280  ; 
    int outheight = 720 ;//2176;  


    int texwidth = 720;//480  ; 
    int texheight = 576;//270 ;//2176; 

  
    FILE * pfile = NULL;
    char layername[100] ;
    long fsize = 0;
    int framecnt = 0;
    void* outmem;
    int i,j;
    struct timeval tpend1, tpend2;
    struct timeval t1, t2;
    struct timeval tm1, tm2;

    long usec1 = 0;
    long usec2 = 0;
    long usec3 = 0;
    
    static void* dso = NULL;
    int writeWidth;
    int writeHeight;
    void *viraddr_in = NULL;
    
    void *viraddr_out = NULL;


    int in_fd = -1;
   
    int out_fd = -1;

    
    static __doByGpuCreate doByGpuCreate = NULL;
    static __doByGpuInit doByGpuInit = NULL;
    static __doByGpuComposition doByGpuComposition = NULL;
    static __doByGpuDestroy doByGpuDestroy = NULL;
    static __doByGpuwaitFence doByGpuwaitFence = NULL;
    static __doByGpucreateFence doByGpucreateFence = NULL;

    viraddr_in = buf_alloc(&in_fd,texwidth,texheight);
    
    viraddr_out = buf_alloc(&out_fd,outwidth,outheight);
    

    printf("rk-debug thead fd[%d,%d],vir[%p,%p] \n",in_fd,out_fd,viraddr_in,viraddr_out);
    memset(viraddr_out,0x00,outwidth*outheight*2);

	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_compress.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_copy_and_compress.bin");
	//sprintf(layername,"/usr/data/s5kgm1sp_2mm_3840x2160_001.nv12");
	//sprintf(layername,"/usr/data/480x270.YUV");
	sprintf(layername,"/usr/data/720x576.yuv");

	pfile = fopen(layername,"rb");
	if(pfile)
	{
        fseek(pfile,0L,SEEK_END);
        int fsize = ftell(pfile);
        fseek(pfile,0L,SEEK_SET);

		int fret= fread((void *)viraddr_in,1,(size_t)(fsize),pfile);
		printf("rk-debug fread =%d,asize=%d\n",fret,fsize );
		fclose(pfile);
	}
    
    if(dso == NULL)
        dso = dlopen(RK_XXX_PATH, RTLD_NOW | RTLD_LOCAL);

    printf("rk_debug dso=%p, name=%s\n",   dso,RK_XXX_PATH);

    if (dso == 0) {
        printf("rk_debug can't not find %s ! error=%s \n",  RK_XXX_PATH, dlerror());
        return 0;
    }




    if(doByGpuCreate == NULL)
        doByGpuCreate = (__doByGpuCreate)dlsym(dso, "doByGpuCreate"); 
    if(doByGpuCreate == NULL)
    {
        printf("rk_debug can't not find doByGpuCreate function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuInit == NULL)
        doByGpuInit = (__doByGpuInit)dlsym(dso, "doByGpuInit"); 
    if(doByGpuInit == NULL)
    {
        printf("rk_debug can't not find doByGpuInit function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    if(doByGpuComposition == NULL)
        doByGpuComposition = (__doByGpuComposition)dlsym(dso, "doByGpuComposition");
    if(doByGpuComposition == NULL)
    {
        printf("rk_debug can't not find doByGpuComposition function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    

    if(doByGpuwaitFence == NULL)
        doByGpuwaitFence = (__doByGpuwaitFence)dlsym(dso, "doByGpuwaitFence");
    if(doByGpuwaitFence == NULL)
    {
        printf("rk_debug can't not find doByGpuwaitFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }


    if(doByGpucreateFence == NULL)
        doByGpucreateFence = (__doByGpucreateFence)dlsym(dso, "doByGpucreateFence");
    if(doByGpucreateFence == NULL)
    {
        printf("rk_debug can't not find doByGpucreateFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuDestroy == NULL)
        doByGpuDestroy = (__doByGpuDestroy)dlsym(dso, "doByGpuDestroy");
    if(doByGpuDestroy == NULL)
    {
        printf("rk_debug can't not find doByGpuDestroy function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    int dotimes = 0;
    int fdoffset = 0;
    
    void *pgl = NULL;
    gettimeofday(&tpend1, NULL);

    pgl = doByGpuCreate();
    printf("rk-debug doByGpuCreate=%p\n",pgl);
    doByGpuInit(pgl,outwidth,outheight,2);        // 初始化

    gettimeofday(&tpend2, NULL);
    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    printf("rk-deubg doByGpuInit use time =%ld ms\n",usec1);        

    gettimeofday(&tm1, NULL);
    
    while(1)
    {
        layer_list_t layerinfo;

        
        memset(&layerinfo,0x00,sizeof(layer_list_t));
        memset(viraddr_out,0x00,outwidth*outheight*2); 
        layerinfo.wfRgb[0] = 1.0;
        layerinfo.wfRgb[1] = 1.0;
        layerinfo.wfRgb[2] = 0.0;
        layerinfo.fcfRgb[0] = 1.0;
        layerinfo.fcfRgb[1] = 0.0;
        layerinfo.fcfRgb[2] = 0.0;
        layerinfo.px = 4;
        layerinfo.pxfc = 6;
        
        dotimes ++;
//        printf("rk-deubg ************************* thread loops [%d] start *************************n",dotimes);        
        usleep(33000);

        if(dotimes >= 20) break; 
        
        gettimeofday(&t1, NULL);
        for(i = 0; i< 8; i++) 
        {
                layerinfo.srcLayerInfo[i].fd = in_fd;
                layerinfo.srcLayerInfo[i].afbc_falg = 0;                
                layerinfo.srcLayerInfo[i].width = texwidth;
                layerinfo.srcLayerInfo[i].height = texheight;
                layerinfo.srcLayerInfo[i].left = 0;
                layerinfo.srcLayerInfo[i].top = 0;
                layerinfo.srcLayerInfo[i].right = texwidth;
                layerinfo.srcLayerInfo[i].bottom = texheight; 
                layerinfo.srcLayerInfo[i].format = DRM_FORMAT_NV12;

                layerinfo.dstLayerInfo[i].fd = out_fd;
                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
                layerinfo.dstLayerInfo[i].width = outwidth;
                layerinfo.dstLayerInfo[i].height = outheight;
                layerinfo.dstLayerInfo[i].left = (outwidth/4) * (i % 4);
                layerinfo.dstLayerInfo[i].top = (outheight/2) * (i / 4);
                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth/4;
                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight/2;    
                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;
            
            if( 1 == i)
            {
                layerinfo.dstLayerInfo[i].focuswin = 1;
            }
            

        }    
        layerinfo.numLayer = 8;
        int ret = doByGpuComposition(pgl,&layerinfo);             

        gettimeofday(&t2, NULL);
        usec1 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg thread frame[%d] GPU process  use time =%ld ms ret=%d\n",dotimes,usec1,ret);    

        gettimeofday(&t1, NULL);
        void *pfence = doByGpucreateFence(pgl);
        gettimeofday(&t2, NULL);
        usec2 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg thread frame[%d] createFence  use time =%ld ms pfence=%p\n",dotimes,usec2,pfence);    

        gettimeofday(&t1, NULL);
        ret = doByGpuwaitFence(pgl,pfence);
        gettimeofday(&t2, NULL);
        usec3 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
        
//        printf("rk-deubg thread frame[%d] doByGpuwaitFence use time =%ld ms ret=%d\n",dotimes,usec3,ret);    
        
        printf("rk-deubg ##############################thread_1 loops [%d] end  total_time=%ld ms ##############################\n",dotimes,usec1 + usec2 + usec3);        

 
        if(4 == dotimes)
        {
            unsigned int *pda = (unsigned int *)viraddr_out;
            for(j = 0 ;j < 64;j++)
                *pda++ = 0x00000000;
            sprintf(layername,"/usr/data/dump/dump_process_thread_test_%dx%d_%d.bin",outwidth,outheight,dotimes);				
            pfile = fopen(layername,"wb");
            printf("rk-debug pfile=%p,layername=%s,line=%d,buf_out=%p\n",pfile,layername,__LINE__,viraddr_out);
            if(pfile)
            {
                fwrite((const void *)viraddr_out,1,(size_t)( outwidth*outheight*4*1.2),pfile); 
                fclose(pfile);
                printf("rk-debug thread----------- write name =%s \n",layername);
            }
        }


    }

    
//    gettimeofday(&tpend1, NULL);
//    doByGpuDestroy(pgl); 
//    gettimeofday(&tpend2, NULL);
//    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
//    printf("rk-deubg thread doByGpuDeinit use time =%ld ms t1\n",usec1);        


}
static void *render_thread2(void *)
{
    int outwidth = 1920  ; 
    int outheight = 1080 ;//2176;  


    int texwidth = 1920;//480  ; 
    int texheight = 1200;//270 ;//2176; 

  
    FILE * pfile = NULL;
    char layername[100] ;
    long fsize = 0;
    int framecnt = 0;
    void* outmem;
    int i,j;
    struct timeval tpend1, tpend2;
    struct timeval t1, t2;
    struct timeval tm1, tm2;

    long usec1 = 0;
    long usec2 = 0;
    long usec3 = 0;
    
    static void* dso = NULL;
    int writeWidth;
    int writeHeight;
    void *viraddr_in = NULL;
     
    void *viraddr_out = NULL;

 
    int in_fd = -1;
   
    int out_fd = -1;

    
    static __doByGpuCreate doByGpuCreate = NULL;
    static __doByGpuInit doByGpuInit = NULL;
    static __doByGpuComposition doByGpuComposition = NULL;
    static __doByGpuDestroy doByGpuDestroy = NULL;
    static __doByGpuwaitFence doByGpuwaitFence = NULL;
    static __doByGpucreateFence doByGpucreateFence = NULL;

    viraddr_in = buf_alloc(&in_fd,texwidth,texheight);
    
    viraddr_out = buf_alloc(&out_fd,outwidth,outheight);
    

    printf("rk-debug thead fd[%d,%d],vir[%p,%p] \n",in_fd,out_fd,viraddr_in,viraddr_out);
    memset(viraddr_out,0x00,outwidth*outheight*2);

	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_compress.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_copy_and_compress.bin");
	//sprintf(layername,"/usr/data/s5kgm1sp_2mm_3840x2160_001.nv12");
	//sprintf(layername,"/usr/data/480x270.YUV");
    sprintf(layername,"/usr/data/1920x1200-nv12.YUV");		

	pfile = fopen(layername,"rb");
	if(pfile)
	{
        fseek(pfile,0L,SEEK_END);
        int fsize = ftell(pfile);
        fseek(pfile,0L,SEEK_SET);

		int fret= fread((void *)viraddr_in,1,(size_t)(fsize),pfile);
		printf("rk-debug fread =%d,asize=%d\n",fret,fsize );
		fclose(pfile);
	}
    
    if(dso == NULL)
        dso = dlopen(RK_XXX_PATH, RTLD_NOW | RTLD_LOCAL);

    printf("rk_debug dso=%p, name=%s\n",   dso,RK_XXX_PATH);

    if (dso == 0) {
        printf("rk_debug can't not find %s ! error=%s \n",  RK_XXX_PATH, dlerror());
        return 0;
    }




    if(doByGpuCreate == NULL)
        doByGpuCreate = (__doByGpuCreate)dlsym(dso, "doByGpuCreate"); 
    if(doByGpuCreate == NULL)
    {
        printf("rk_debug can't not find doByGpuCreate function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuInit == NULL)
        doByGpuInit = (__doByGpuInit)dlsym(dso, "doByGpuInit"); 
    if(doByGpuInit == NULL)
    {
        printf("rk_debug can't not find doByGpuInit function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    if(doByGpuComposition == NULL)
        doByGpuComposition = (__doByGpuComposition)dlsym(dso, "doByGpuComposition");
    if(doByGpuComposition == NULL)
    {
        printf("rk_debug can't not find doByGpuComposition function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    

    if(doByGpuwaitFence == NULL)
        doByGpuwaitFence = (__doByGpuwaitFence)dlsym(dso, "doByGpuwaitFence");
    if(doByGpuwaitFence == NULL)
    {
        printf("rk_debug can't not find doByGpuwaitFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }


    if(doByGpucreateFence == NULL)
        doByGpucreateFence = (__doByGpucreateFence)dlsym(dso, "doByGpucreateFence");
    if(doByGpucreateFence == NULL)
    {
        printf("rk_debug can't not find doByGpucreateFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuDestroy == NULL)
        doByGpuDestroy = (__doByGpuDestroy)dlsym(dso, "doByGpuDestroy");
    if(doByGpuDestroy == NULL)
    {
        printf("rk_debug can't not find doByGpuDestroy function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    int dotimes = 0;
    int fdoffset = 0;
    
    void *pgl = NULL;
    gettimeofday(&tpend1, NULL);

    pgl = doByGpuCreate();
    printf("rk-debug doByGpuCreate=%p\n",pgl);
    doByGpuInit(pgl,outwidth,outheight,0);        // 初始化

    gettimeofday(&tpend2, NULL);
    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    printf("rk-deubg doByGpuInit use time =%ld ms\n",usec1);        

    gettimeofday(&tm1, NULL);
    
    while(1)
    {
        layer_list_t layerinfo;

        
        memset(&layerinfo,0x00,sizeof(layer_list_t));
        memset(viraddr_out,0x00,outwidth*outheight*2); 
        layerinfo.wfRgb[0] = 1.0;
        layerinfo.wfRgb[1] = 1.0;
        layerinfo.wfRgb[2] = 0.0;
        layerinfo.fcfRgb[0] = 1.0;
        layerinfo.fcfRgb[1] = 0.0;
        layerinfo.fcfRgb[2] = 0.0;
        layerinfo.px = 4;
        layerinfo.pxfc = 6;
        
        dotimes ++;
//        printf("rk-deubg ************************* thread loops [%d] start *************************n",dotimes);        
        usleep(33000);

        if(dotimes >= 20) break; 
        
        gettimeofday(&t1, NULL);
        for(i = 0; i< 8; i++) 
        {
                layerinfo.srcLayerInfo[i].fd = in_fd;
                layerinfo.srcLayerInfo[i].afbc_falg = 0;                
                layerinfo.srcLayerInfo[i].width = texwidth;
                layerinfo.srcLayerInfo[i].height = texheight;
                layerinfo.srcLayerInfo[i].left = 0;
                layerinfo.srcLayerInfo[i].top = 0;
                layerinfo.srcLayerInfo[i].right = texwidth;
                layerinfo.srcLayerInfo[i].bottom = texheight; 
                layerinfo.srcLayerInfo[i].format = DRM_FORMAT_NV12;

                layerinfo.dstLayerInfo[i].fd = out_fd;
                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
                layerinfo.dstLayerInfo[i].width = outwidth;
                layerinfo.dstLayerInfo[i].height = outheight;
                layerinfo.dstLayerInfo[i].left = (outwidth/4) * (i % 4);
                layerinfo.dstLayerInfo[i].top = (outheight/2) * (i / 4);
                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth/4;
                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight/2;    
                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;
            
            if( 4 == i)
            {
                layerinfo.dstLayerInfo[i].focuswin = 1;
            }
            

        }    
        layerinfo.numLayer = 8;
        int ret = doByGpuComposition(pgl,&layerinfo);             

        gettimeofday(&t2, NULL);
        usec1 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg thread frame[%d] GPU process  use time =%ld ms ret=%d\n",dotimes,usec1,ret);    

        gettimeofday(&t1, NULL);
        void *pfence = doByGpucreateFence(pgl);
        gettimeofday(&t2, NULL);
        usec2 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg thread frame[%d] createFence  use time =%ld ms pfence=%p\n",dotimes,usec2,pfence);    

        gettimeofday(&t1, NULL);
        ret = doByGpuwaitFence(pgl,pfence);
        gettimeofday(&t2, NULL);
        usec3 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
        
//        printf("rk-deubg thread frame[%d] doByGpuwaitFence use time =%ld ms ret=%d\n",dotimes,usec3,ret);    
        
        printf("rk-deubg ##############################thread_2 loops [%d] end  total_time=%ld ms ##############################\n",dotimes,usec1 + usec2 + usec3);        

 
//        if(4 == dotimes)
//        {
//            unsigned int *pda = (unsigned int *)viraddr_out;
//            for(j = 0 ;j < 64;j++)
//                *pda++ = 0x00000000;
//            sprintf(layername,"/usr/data/dump/dump_process_thread_test_%dx%d_%d.bin",outwidth,outheight,dotimes);				
//            pfile = fopen(layername,"wb");
//            printf("rk-debug pfile=%p,layername=%s,line=%d,buf_out=%p\n",pfile,layername,__LINE__,viraddr_out);
//            if(pfile)
//            {
//                fwrite((const void *)viraddr_out,1,(size_t)( outwidth*outheight*4*1.2),pfile); 
//                fclose(pfile);
//                printf("rk-debug thread2----------- write name =%s \n",layername);
//            }
//        }


    }

    
//    gettimeofday(&tpend1, NULL);
//    doByGpuDestroy(pgl); 
//    gettimeofday(&tpend2, NULL);
//    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
//    printf("rk-deubg thread doByGpuDeinit use time =%ld ms t1\n",usec1);        


}

int main(int argc, char** argv) 
{
    int mapWidth = 8;//3840;//121;
    int mapHeight = 8;//2160;//137;
    int outwidth = 1920 *2 ; 
    int outheight = 1080 *2 ;//2176;  

    int outwidth2 = 1920 ; 
    int outheight2 = 1080 ;//2176;  

    int texwidth = 720;//480  ; 
    int texheight = 576;//270 ;//2176; 
    int texwidth2 = 1920 ; 
    int texheight2 = 1200;//1200;//2176; 
    int tw = 1280;//480  ; 
    int th = 720;//270 ;//2176; 

  
    FILE * pfile = NULL;
    char layername[100] ;
    long fsize = 0;
    int framecnt = 0;
    void* outmem;
    int i,j;
    struct timeval tpend1, tpend2;
    struct timeval t1, t2;
    struct timeval tm1, tm2;

    long usec1 = 0;
    long usec2 = 0;
    long usec3 = 0;
    
    static void* dso = NULL;
    int writeWidth;
    int writeHeight;
    void *viraddr_in = NULL;
    void *viraddr_in2 = NULL;
    void *viraddr_inarry[MaxTex];
    
    void *viraddr_out = NULL;
    void *viraddr_out2 = NULL;


    int in_fd = -1;
    int in_fd2 = -1;
    int in_fdarry[MaxTex];
   
    int out_fd = -1;
    int out_fd2 = -1;

    pthread_t   tid;
    pthread_t   tid2;

    static __doByGpuCreate doByGpuCreate = NULL;
    static __doByGpuInit doByGpuInit = NULL;
    static __doByGpuComposition doByGpuComposition = NULL;
    static __doByGpuDestroy doByGpuDestroy = NULL;
    static __doByGpuwaitFence doByGpuwaitFence = NULL;
    static __doByGpucreateFence doByGpucreateFence = NULL;

    viraddr_in = buf_alloc(&in_fd,texwidth,texheight);
    viraddr_in2 = buf_alloc(&in_fd2,texwidth2,texheight2);
    
    viraddr_out = buf_alloc(&out_fd,outwidth,outheight);
    viraddr_out2 = buf_alloc(&out_fd2,outwidth,outheight);
    
    for( i = 0; i < MaxTex ; i++ )
    {
        viraddr_inarry[i] = NULL;
        in_fdarry[i] = -1;
        viraddr_inarry[i] = buf_alloc(&in_fdarry[i],tw,th);
        if(NULL == viraddr_inarry[i])
        {
            printf("rk-debug alloc failed i=%d\n,[%d,%d]",i,tw,th);
        }
        sprintf(layername,"/usr/data/1280x720.YUV");

        pfile = fopen(layername,"rb");
        if(pfile)
        {
            fseek(pfile,0L,SEEK_END);
            int fsize = ftell(pfile);
            fseek(pfile,0L,SEEK_SET);
            int fret= fread((void *)viraddr_inarry[i],1,(size_t)(fsize),pfile);
            fclose(pfile);
        }


    }
    
//    pthread_create(&tid, NULL, render_thread, NULL);
//    pthread_create(&tid2, NULL, render_thread2, NULL);

    printf("rk-debug fd[%d,%d],vir[%p,%p] \n",in_fd,out_fd,viraddr_in,viraddr_out);
    memset(viraddr_out,0x00,outwidth*outheight*2);

	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_compress.bin");
	//sprintf(layername,"/usr/data/3840x2176_8bit_420_head_vir_word_str_960_copy_and_compress.bin");
	//sprintf(layername,"/usr/data/s5kgm1sp_2mm_3840x2160_001.nv12");
	//sprintf(layername,"/usr/data/480x270.YUV");
	sprintf(layername,"/usr/data/720x576.yuv");

	pfile = fopen(layername,"rb");
	if(pfile)
	{
        fseek(pfile,0L,SEEK_END);
        int fsize = ftell(pfile);
        fseek(pfile,0L,SEEK_SET);

		int fret= fread((void *)viraddr_in,1,(size_t)(fsize),pfile);
		printf("rk-debug fread =%d,asize=%d\n",fret,fsize );
		fclose(pfile);
	}

//    sprintf(layername,"/usr/data/960x540.YUV");		
    sprintf(layername,"/usr/data/1920x1200-nv12.YUV");		
//    sprintf(layername,"/usr/data/1920x1080_5_nv12_afbc.yuv");		

	//sprintf(layername,"/usr/data/dump/dump_gpu_process_test_960x540_8_afbc.bin");		
	
	pfile = fopen(layername,"rb");
	if(pfile)
	{
        fseek(pfile,0L,SEEK_END);
        int fsize = ftell(pfile);
        fseek(pfile,0L,SEEK_SET);

		int fret= fread((void *)viraddr_in2,1,(size_t)(fsize),pfile);
		printf("rk-debug fread 2 =%d,asize=%d,name=%s\n",fret,fsize ,layername);
		fclose(pfile);
	}
    
    if(dso == NULL)
        dso = dlopen(RK_XXX_PATH, RTLD_NOW | RTLD_LOCAL);

    printf("rk_debug dso=%p, name=%s\n",   dso,RK_XXX_PATH);

    if (dso == 0) {
        printf("rk_debug can't not find %s ! error=%s \n",  RK_XXX_PATH, dlerror());
        return 0;
    }




    if(doByGpuCreate == NULL)
        doByGpuCreate = (__doByGpuCreate)dlsym(dso, "doByGpuCreate"); 
    if(doByGpuCreate == NULL)
    {
        printf("rk_debug can't not find doByGpuCreate function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuInit == NULL)
        doByGpuInit = (__doByGpuInit)dlsym(dso, "doByGpuInit"); 
    if(doByGpuInit == NULL)
    {
        printf("rk_debug can't not find doByGpuInit function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    if(doByGpuComposition == NULL)
        doByGpuComposition = (__doByGpuComposition)dlsym(dso, "doByGpuComposition");
    if(doByGpuComposition == NULL)
    {
        printf("rk_debug can't not find doByGpuComposition function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }
    

    if(doByGpuwaitFence == NULL)
        doByGpuwaitFence = (__doByGpuwaitFence)dlsym(dso, "doByGpuwaitFence");
    if(doByGpuwaitFence == NULL)
    {
        printf("rk_debug can't not find doByGpuwaitFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }


    if(doByGpucreateFence == NULL)
        doByGpucreateFence = (__doByGpucreateFence)dlsym(dso, "doByGpucreateFence");
    if(doByGpucreateFence == NULL)
    {
        printf("rk_debug can't not find doByGpucreateFence function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    if(doByGpuDestroy == NULL)
        doByGpuDestroy = (__doByGpuDestroy)dlsym(dso, "doByGpuDestroy");
    if(doByGpuDestroy == NULL)
    {
        printf("rk_debug can't not find doByGpuDestroy function in /system/lib64/libbicubic_gl.so ! \n");
        dlclose(dso);
        return 0;
    }

    int dotimes = 0;
    int fdoffset = 0;
    
    void *pgl = NULL;
    gettimeofday(&tpend1, NULL);

    pgl = doByGpuCreate();
    printf("rk-debug doByGpuCreate=%p\n",pgl);
    doByGpuInit(pgl,outwidth,outheight,0);        // 初始化

    gettimeofday(&tpend2, NULL);
    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    printf("rk-deubg doByGpuInit use time =%ld ms\n",usec1);        

    gettimeofday(&tm1, NULL);

    while(1)
    {
        layer_list_t layerinfo;

        
        memset(&layerinfo,0x00,sizeof(layer_list_t));
        memset(viraddr_out,0x00,outwidth*outheight*2); 
        memset(viraddr_out2,0x00,outwidth*outheight*2); 
        layerinfo.wfRgb[0] = 1.0;
        layerinfo.wfRgb[1] = 1.0;
        layerinfo.wfRgb[2] = 0.0;
        layerinfo.fcfRgb[0] = 1.0;
        layerinfo.fcfRgb[1] = 0.0;
        layerinfo.fcfRgb[2] = 0.0;
        layerinfo.px = 6;
        layerinfo.pxfc = 6;
        
        dotimes ++;
//        printf("rk-deubg ############################## loops [%d] start ##############################\n",dotimes);        
        usleep(33000);
        if(dotimes >= 10) break; 
        
        gettimeofday(&t1, NULL);
        for(i = 0; i< 4; i++) 
        {
//            if(i%2)
//           // if(0)
//            {
//                layerinfo.srcLayerInfo[i].fd = in_fd;
//                layerinfo.srcLayerInfo[i].afbc_falg = 0;                
//                layerinfo.srcLayerInfo[i].width = texwidth;
//                layerinfo.srcLayerInfo[i].height = texheight;
//                layerinfo.srcLayerInfo[i].left = 0;
//                layerinfo.srcLayerInfo[i].top = 0;
//                layerinfo.srcLayerInfo[i].right = texwidth;
//                layerinfo.srcLayerInfo[i].bottom = texheight; 
//                layerinfo.srcLayerInfo[i].format = DRM_FORMAT_NV12;

//            }
//            else
            {
                layerinfo.srcLayerInfo[i].fd = in_fd2;
                layerinfo.srcLayerInfo[i].afbc_falg = 0;
                layerinfo.srcLayerInfo[i].width = texwidth2 ;
                layerinfo.srcLayerInfo[i].height = texheight2 ;
                layerinfo.srcLayerInfo[i].left = 0;
                layerinfo.srcLayerInfo[i].top = 0;
                layerinfo.srcLayerInfo[i].right = texwidth2;
                layerinfo.srcLayerInfo[i].bottom = texheight2; 
                layerinfo.srcLayerInfo[i].format = DRM_FORMAT_NV12;
                layerinfo.srcLayerInfo[i].color_space = EGL_ITU_REC2020_EXT;
                layerinfo.srcLayerInfo[i].sample_range = EGL_YUV_NARROW_RANGE_EXT;
                
            }
//            layerinfo.srcLayerInfo[i].fd = in_fdarry[fdoffset%MaxTex];
//            layerinfo.srcLayerInfo[i].afbc_falg = 0;                
//            layerinfo.srcLayerInfo[i].width = tw;
//            layerinfo.srcLayerInfo[i].height = th;
//            layerinfo.srcLayerInfo[i].left = 0;
//            layerinfo.srcLayerInfo[i].top = 0;
//            layerinfo.srcLayerInfo[i].right = tw;
//            layerinfo.srcLayerInfo[i].bottom = th; 
//            layerinfo.srcLayerInfo[i].format = DRM_FORMAT_NV12;
//            fdoffset ++;
//            if(dotimes%2)
//            {
//                layerinfo.dstLayerInfo[i].fd = out_fd;
//                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
//                layerinfo.dstLayerInfo[i].width = outwidth;
//                layerinfo.dstLayerInfo[i].height = outheight;
//                layerinfo.dstLayerInfo[i].left = (outwidth/4) * (i % 4);
//                layerinfo.dstLayerInfo[i].top = (outheight/2) * (i / 4);
//                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth/4;
//                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight/2;    
//                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;
//                
//            } 
//            else
            #if 0
            {
                layerinfo.dstLayerInfo[i].fd = out_fd2;
                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
                layerinfo.dstLayerInfo[i].width = outwidth2;
                layerinfo.dstLayerInfo[i].height = outheight2;
                layerinfo.dstLayerInfo[i].left = (outwidth2/4) * (i % 4);
                layerinfo.dstLayerInfo[i].top = (outheight2/2) * (i / 4);
                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth2/4;
                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight2/2;    
                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;    
                layerinfo.dstLayerInfo[i].color_space = EGL_ITU_REC709_EXT;
                layerinfo.dstLayerInfo[i].sample_range = EGL_YUV_NARROW_RANGE_EXT;
            
            } 
            #else   // 不规则显示矩形
            if(0 == i || 1 == i )
            {
                layerinfo.dstLayerInfo[i].fd = out_fd2;
                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
                layerinfo.dstLayerInfo[i].width = outwidth2;
                layerinfo.dstLayerInfo[i].height = outheight2;
                layerinfo.dstLayerInfo[i].left = (outwidth2/3) * i;
                layerinfo.dstLayerInfo[i].top = 0;
                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth2/3;
                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight2/3;    
                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;    
                layerinfo.dstLayerInfo[i].color_space = EGL_ITU_REC709_EXT;
                layerinfo.dstLayerInfo[i].sample_range = EGL_YUV_NARROW_RANGE_EXT;
            
            } 
            else if(2 == i || 3 == i )
            {
                layerinfo.dstLayerInfo[i].fd = out_fd2;
                layerinfo.dstLayerInfo[i].afbc_falg = 0; 
                layerinfo.dstLayerInfo[i].width = outwidth2;
                layerinfo.dstLayerInfo[i].height = outheight2;
                layerinfo.dstLayerInfo[i].left = (outwidth2/3) * (i -2);
                layerinfo.dstLayerInfo[i].top = (outheight2/2) ;
                layerinfo.dstLayerInfo[i].right = layerinfo.dstLayerInfo[i].left + outwidth2/3;
                layerinfo.dstLayerInfo[i].bottom = layerinfo.dstLayerInfo[i].top + outheight2/3;    
                layerinfo.dstLayerInfo[i].format = DRM_FORMAT_ABGR8888;//DRM_FORMAT_NV12;// DRM_FORMAT_BGR888;//DRM_FORMAT_ABGR8888;    
                layerinfo.dstLayerInfo[i].color_space = EGL_ITU_REC709_EXT;
                layerinfo.dstLayerInfo[i].sample_range = EGL_YUV_NARROW_RANGE_EXT;
            
            } 
            
            #endif
            if( 2 == i)
            {
                layerinfo.dstLayerInfo[i].focuswin = 1;
            }
            

        }    
        layerinfo.numLayer = 4;
        int ret = doByGpuComposition(pgl,&layerinfo);             

        gettimeofday(&t2, NULL);
        usec1 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg frame[%d] GPU process  use time =%ld ms ret=%d\n",dotimes,usec1,ret);    

        gettimeofday(&t1, NULL);
        void *pfence = doByGpucreateFence(pgl);
        gettimeofday(&t2, NULL);
        usec2 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
//        printf("rk-deubg frame[%d] createFence  use time =%ld ms pfence=%p\n",dotimes,usec2,pfence);    

        gettimeofday(&t1, NULL);
        ret = doByGpuwaitFence(pgl,pfence);
        gettimeofday(&t2, NULL);
        usec3 = 1000 * (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000;
        
//        printf("rk-deubg frame[%d] doByGpuwaitFence use time =%ld ms ret=%d\n",dotimes,usec3,ret);    
        
        printf("rk-deubg ##############################thread_0 loops [%d] end  total_time=%ld ms ##############################\n",dotimes,usec1 + usec2 + usec3);        
// 
//        if(3 == dotimes)
//        {

//            sprintf(layername,"/usr/data/dump/dump_gpu_process_test_%dx%d_%d.bin",outwidth,outheight,dotimes);				
//            pfile = fopen(layername,"wb");
//            printf("rk-debug pfile=%p,layername=%s,line=%d,buf_out=%p\n",pfile,layername,__LINE__,viraddr_out);
//            if(pfile)
//            {
//                fwrite((const void *)viraddr_out,1,(size_t)( outwidth*outheight*4*1.2),pfile); 
//                fclose(pfile);
//                printf("rk-debug ----------- write name =%s \n",layername);
//            }
//        }

 
        if(5 == dotimes)
        {
            unsigned int *pda = (unsigned int *)viraddr_out2;
            for(j = 0 ;j < 64;j++)
                *pda++ = 0x00000000;
            sprintf(layername,"/usr/data/dump/dump_process_test_%dx%d_%d.bin",outwidth2,outheight2,dotimes);				
            pfile = fopen(layername,"wb");
            printf("rk-debug pfile=%p,layername=%s,line=%d,buf_out=%p\n",pfile,layername,__LINE__,viraddr_out2);
            if(pfile)
            {
                fwrite((const void *)viraddr_out2,1,(size_t)( outwidth2*outheight2*4*1.2),pfile); 
                fclose(pfile);
                printf("rk-debug ----------- write name =%s \n",layername);
            }
        }


    }

    int  res = pthread_join(tid, NULL);  
//    res = pthread_join(tid2, NULL);  

    gettimeofday(&tm2, NULL);

    usec1 =  (tm2.tv_sec - tm1.tv_sec) ;
    printf("rk-deubg total run use time =%ld s\n",usec1);        
    
    gettimeofday(&tpend1, NULL);
    doByGpuDestroy(pgl); 
    gettimeofday(&tpend2, NULL);
    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    printf("rk-deubg doByGpuDeinit use time =%ld ms t1\n",usec1);        


    return 0;
    
}
