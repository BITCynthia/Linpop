/*
#####################################################
#                      Linpop                       #
#                                                   #
#                               user  id :          #
#                                password :         #
#                                                   #
#               login              exit             #
#####################################################
 */
 /***********************************************************************
*                         登录                                          *
*                            主界面                                      *
*                               聊天                                    *
*                                  ……                                   *
*                                    连接                               *
*                                     Developed by Housifan ,Duwenyi   *
************************************************************************/
 #include "linpop.h"
 #include "linpopwnd.h"
 
 extern GData *widget_chat_dlg;// 聊天窗口集, 用于获取已经建立的聊天窗口
 
 GArray *g_array_user = NULL; // 储存用户信息的全局变量
 
 GtkWidget *login_window, *linpop_window;
 GtkWidget *vbox_login;
 int utimer_login, utimer_count = 0;
 
 GtkWidget *g_treeview;
 
 extern GtkWidget *button_image, *button_preview;
 extern GtkWidget *image_user, *image_user_old, *image_user_new;
 extern gchar *str_image_user, *str_image_user_new;
 
 GtkWidget *entry_user_id, *entry_pass; // 用户 id 和密码输入 entry
 char my_id[15];
 
 int socket_ret;// socket 连接 server 返回值
 
 gboolean thread_quit = FALSE; // 主窗口退出时退出 recv_msg thread
 
 
 // 同步所用, 主窗口创建后再收到用户在线数据
 int gnum = 0;
 pthread_mutex_t mutex; // 互斥量
 pthread_cond_t cond; // 条件变量
 
 
 gboolean gtk_wnd_init(int argc, char *argv[])
 {
	 // 同步所用
	 pthread_mutex_init(&mutex, NULL); // 互斥初始化
	 pthread_cond_init(&cond, NULL); // 条件变量初始化
 
	 gtk_init(&argc, &argv);
 
	 login_window = create_login_wnd();
 
	 if (!g_thread_supported())
		 g_thread_init(NULL);
 
	 gdk_threads_init();
 
	 // 关键之处, gdk_threads_enter 和 gdk_threads_leave, 否则在线程中创建窗口会崩溃
 
	 gdk_threads_enter();
	 gtk_main();
	 gdk_threads_leave();
 
	 return TRUE;
 }
 
 
 static void set_widget_font_size(GtkWidget *widget, int size, gboolean is_button)
 {
	 GtkWidget *labelChild;  
	 PangoFontDescription *font;  
	 gint fontSize = size;  
 
	 font = pango_font_description_from_string("Sans");          //"Sans"字体名   
	 pango_font_description_set_size(font, fontSize*PANGO_SCALE);//设置字体大小   
 
	 if(is_button){
		 labelChild = gtk_bin_get_child(GTK_BIN(widget));//取出GtkButton里的label  
	 }else{
		 labelChild = widget;
	 }
 
	 //设置label的字体，这样这个GtkButton上面显示的字体就变了
	 gtk_widget_modify_font(GTK_WIDGET(labelChild), font);
	 pango_font_description_free(font);
 }
 
 
 GtkWidget *create_login_wnd()//登录主窗口
 {
	 //	GtkWidget *entry_user_id, *entry_pass; // 提取到全局变量
	 GtkWidget *label,*label1;
	 GtkWidget *button_login,*button_exit;
	 GtkWidget *hbox_user_id, *hbox_pass, *hbox_button;
	 GtkWidget *align;
 
	 // 创建登录主窗口
	 login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	 gtk_window_set_title(GTK_WINDOW(login_window), "Linpop即时通");
	 gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER);
	 //gtk_widget_set_size_request(login_window, 285, 625);
	 gtk_window_set_resizable(GTK_WINDOW(login_window),FALSE);//窗体大小固定
	 gtk_container_set_border_width(GTK_CONTAINER (login_window), 10);
	 chang_background(login_window, 400, 300, "background.jpg");//设置登录页面背景函数
	 gtk_window_set_decorated(GTK_WINDOW(login_window), FALSE);       // 设置无边框 
	 gtk_window_set_opacity(GTK_WINDOW(login_window), 0.88);
 
	 vbox_login = gtk_vbox_new(FALSE, 20);
	 gtk_container_set_border_width(GTK_CONTAINER (vbox_login), 30);
 
		 label1 = gtk_label_new("                                           ");
		 set_widget_font_size(label1, 20, FALSE);
		 gtk_box_pack_start(GTK_BOX(vbox_login),label1,TRUE,FALSE,0);
 
	 label = gtk_label_new("                 ");
	 entry_user_id = gtk_entry_new();
	 gtk_entry_set_text(GTK_ENTRY (entry_user_id), " User  ID");//改为User ID
	 hbox_user_id = gtk_hbox_new(FALSE, 0);
	 gtk_box_pack_start(GTK_BOX (hbox_user_id), label, TRUE, FALSE, 2);
	 gtk_box_pack_start(GTK_BOX (hbox_user_id), entry_user_id, TRUE, FALSE, 0);
 
	 label = gtk_label_new("                 ");
	 entry_pass = gtk_entry_new();
	 gtk_entry_set_text(GTK_ENTRY (entry_pass), "******");
	 gtk_entry_set_visibility(GTK_ENTRY (entry_pass), FALSE);
	 gtk_entry_set_invisible_char(GTK_ENTRY (entry_pass), '*');
 
	 // 响应回车消息
	 g_signal_connect (G_OBJECT (entry_user_id), "activate",
			 G_CALLBACK (enter_callback), NULL);
	 g_signal_connect (G_OBJECT (entry_pass), "activate",
			 G_CALLBACK (enter_callback), NULL);
 
	 hbox_pass = gtk_hbox_new(FALSE, 5);
	 gtk_box_pack_start(GTK_BOX (hbox_pass), label, TRUE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX (hbox_pass), entry_pass, TRUE, FALSE, 0);
 
	 button_login = gtk_button_new_with_label("   Login   ");
	 button_exit = gtk_button_new_with_label("    Exit    ");
	 g_signal_connect(G_OBJECT(button_login), "clicked",G_CALLBACK(login), NULL);
	 g_signal_connect(G_OBJECT(button_exit),"clicked",G_CALLBACK(gtk_main_quit),NULL);
	 hbox_button = gtk_hbox_new(FALSE, 5);
 
	 // 创建一个居中对齐的对象
	 align = gtk_alignment_new(0.5, 0.8, 0, 0);
	 gtk_box_pack_start(GTK_BOX (hbox_button), align, TRUE, TRUE, 5);
	 gtk_widget_show(align);
	 gtk_container_add(GTK_CONTAINER (align), button_login);
	 gtk_box_pack_start(GTK_BOX (hbox_button), button_exit, TRUE, FALSE, 10); 
 
	 gtk_box_pack_start(GTK_BOX (vbox_login), hbox_user_id, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX (vbox_login), hbox_pass, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX (vbox_login), hbox_button, FALSE, FALSE, 0);

	 gtk_container_add(GTK_CONTAINER(login_window), vbox_login);
 
	 g_signal_connect (G_OBJECT (login_window), "destroy",
			 G_CALLBACK (gtk_main_quit), NULL);
 
	 gtk_widget_show_all(login_window);
 
	 return login_window;
 }
 
 GtkWidget *create_linpopwnd()//好友列表
 {
	 //	GtkWidget *treeview;
	 GtkWidget *label_info;
	 // 便于修改头像
	 GtkWidget *scrolled_win; // tree view 自动滚动
	 GtkWidget *vbox_main, *hbox_head,*button_ca,*hbox_middle;//添加日历
 
	 // 创建主窗口
	 linpop_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	 gtk_window_set_title(GTK_WINDOW(linpop_window), "Linpop");
	 g_signal_connect (G_OBJECT (linpop_window), "destroy",G_CALLBACK (linpop_quit), NULL);
	 //gtk_window_set_resizable(GTK_WINDOW(linpop_window),FALSE);//窗体大小自适应
	 gtk_widget_set_size_request(linpop_window, 300, 680);
	 chang_background(linpop_window, 400, 700, "background2.jpeg");//设置好友页面背景函数
	 gtk_window_set_opacity(GTK_WINDOW(linpop_window), 0.9);
	 gtk_window_set_position(GTK_WINDOW(linpop_window), GTK_WIN_POS_NONE);
 
	 vbox_main = gtk_vbox_new(FALSE, 10);
	 hbox_head = gtk_hbox_new(FALSE, 10);
	 hbox_middle = gtk_hbox_new(FALSE,5);//添加横框
 
	 // 用户头像
	 image_user = gtk_image_new_from_file("./Image/newface/1.bmp");
	 // 保存原来头像信息
	 str_image_user = g_strdup("./Image/newface/1.bmp");
	 button_image = gtk_button_new();
	 label_info = gtk_label_new("         Developed by group 1 .\n         All rights reserved .");
         set_widget_font_size(label_info, 50, TRUE);
	 gtk_container_add(GTK_CONTAINER(button_image), (GtkWidget *) image_user);
	 g_signal_connect(G_OBJECT(button_image), "clicked",
			 G_CALLBACK(set_user_info), NULL);
	 gtk_widget_set_size_request(button_image, 64, 64);
 
	 button_ca = gtk_button_new_with_label("Calendar");//显示日历按钮
	 set_widget_font_size(button_ca, 10, TRUE);//设置大小
	 g_signal_connect(G_OBJECT(button_ca), "clicked",G_CALLBACK(create_calendar), NULL);
 ///////////////////////////////////////////////////////////////////////////
 GtkWidget *button_game;//添加游戏。。。。。。。。。。。。。。。。。。。。。。。。。
 button_game = gtk_button_new_with_label("Game");//显示游戏按钮
 set_widget_font_size(button_game, 10, TRUE);//设置大小
 g_signal_connect(G_OBJECT(button_game), "clicked",G_CALLBACK(create_game), NULL);
///////////////////////////////////////////////////////////////////////////
	 // 头像和文本信息
	 gtk_box_pack_start(GTK_BOX(hbox_head), button_image, FALSE, FALSE, 0);
	 gtk_box_pack_start(GTK_BOX(hbox_head), label_info, FALSE, FALSE, 5);
	 gtk_box_pack_start(GTK_BOX(hbox_middle), button_ca, FALSE, FALSE, 5);//放入横框
	 gtk_box_pack_end(GTK_BOX(hbox_middle), button_game, FALSE, FALSE, 5);//放入游戏
 
	 // 创建树视图
	 g_treeview = create_tree_view();
	 setup_tree_view_model(g_treeview, NULL, "");
 
	 scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_win),
			 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	 gtk_container_add(GTK_CONTAINER (scrolled_win), g_treeview);
 
	 gtk_container_add(GTK_CONTAINER(linpop_window), vbox_main);
	 gtk_box_pack_start(GTK_BOX(vbox_main), hbox_head, FALSE, FALSE, 2);
	 gtk_box_pack_start(GTK_BOX(vbox_main), scrolled_win, TRUE, TRUE, 1);
	 gtk_box_pack_start(GTK_BOX(vbox_main), hbox_middle, FALSE, FALSE, 1);//设置在下方
	 // 显示主窗口
	 gtk_widget_show_all(linpop_window);
 
	 return linpop_window;
 }
 
 void enter_callback(GtkWidget *widget, GtkWidget *entry)
 {
	 const char *text_id, *text_pass;
	 text_id = gtk_entry_get_text(GTK_ENTRY (entry_user_id));
	 text_pass = gtk_entry_get_text(GTK_ENTRY (entry_pass));
 
	 login(login_window, NULL);
 }
 
 void login(GtkWidget *widget, gpointer data)
 {
	 const char *text_id, *text_pass;
	 text_id = gtk_entry_get_text(GTK_ENTRY (entry_user_id));
	 text_pass = gtk_entry_get_text(GTK_ENTRY (entry_pass));
 
	 strcpy(my_id, text_id);
 
	 if (text_id == NULL || strlen(text_id) == 0)
	 {
		 show_info_msg_box(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK,
				 "Please entry your user ID!");
		 return;
	 }
	 else if (text_pass == NULL || strlen(text_pass) == 0)
	 {
		 show_info_msg_box(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK,
				 "Please entry your password!");
		 return;
	 }
	else if(strcmp(text_id,text_pass)!=0)
	{
		show_info_msg_box(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK,
				 "Invalid password!");
		 return;
	}
 
	 // 连接服务器, 进行登录
 
	 char server_ip[16];
	 memset(server_ip, 0, sizeof(server_ip));
	 int server_port;
	 if (access("server.conf", 0) == -1)
	 {
		 show_info_msg_box(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				 "File server.conf not exist!");
		 return;
	 }
 
	 FILE *fp = fopen("server.conf", "rb");
	 fscanf(fp, "%s %d", server_ip, &server_port);
	 fclose(fp);
 
	 socket_ret = init_socket(server_ip, server_port);
 
	 // 要求服务器传回上线用户列表
	 char *pro_buf = g_strdup(PROTOCOL_ONLINE); // 要求服务器传回在线用户
	 gchar *msg = g_strconcat(pro_buf, text_id, NULL);
	 socket_ret = send_msg(msg, strlen(msg));
 
	 GtkWidget *progress;
	 GtkWidget *vbox_progress, *align;
 
	 gtk_container_remove(GTK_CONTAINER (login_window), vbox_login);
	 vbox_progress = gtk_vbox_new(FALSE, 20);//0--->20
	 gtk_container_set_border_width(GTK_CONTAINER (vbox_progress), 30);//0--->30
	 //gtk_container_add(GTK_CONTAINER (login_window), vbox_progress);
	 //gtk_widget_show(vbox_progress);//............................修改
 
////////////////////////////////////////////////////////////////////////////
    //connecting。。。。。。
	GtkWidget *label;
	label = gtk_label_new("                                           ");
	set_widget_font_size(label, 20, FALSE);
	gtk_box_pack_start(GTK_BOX (vbox_progress), label, TRUE, FALSE, 0);
	//居中对齐的对象
	GtkWidget *hbox_progress;
	hbox_progress= gtk_hbox_new(FALSE, 5);
	align = gtk_alignment_new(0.5, 0.25, 0, 0);
	gtk_box_pack_start(GTK_BOX (hbox_progress), align, TRUE, TRUE, 5);
	gtk_widget_show(align);
	//进度条
	progress = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress, 280, 10);
	utimer_login = gtk_timeout_add(60, (GtkFunction) progress_login_timeout,progress);
	gtk_container_add(GTK_CONTAINER(align),progress);
	gtk_widget_show(progress);

	gtk_box_pack_start(GTK_BOX (vbox_progress), hbox_progress, FALSE, FALSE, 0);	

	gtk_container_add(GTK_CONTAINER (login_window), vbox_progress);
	gtk_widget_show_all(login_window);
////////////////////////////////////////////////////////////////////////////
/*
	 // 创建一个居中对齐的对象
	 align = gtk_alignment_new(0.5, 0.25, 0, 0);
	 gtk_box_pack_start(GTK_BOX (vbox_progress), align, TRUE, TRUE, 5);
	 gtk_widget_show(align);
 
	 progress = gtk_progress_bar_new();
	 gtk_widget_set_size_request(progress, 200, 10);
	 utimer_login = gtk_timeout_add(60, (GtkFunction) progress_login_timeout,
			 progress);
 
	 gtk_container_add(GTK_CONTAINER (align), progress);
	 gtk_widget_show(progress);
	 */
 }
 
 gboolean progress_login_timeout(GtkProgressBar *progress)
 {
	 utimer_count++;
	 gtk_progress_bar_pulse(GTK_PROGRESS_BAR (progress));
 
	 if (utimer_count >= 20 && socket_ret >= 0)
	 {
		 gtk_timeout_remove(utimer_login);
		 gtk_widget_hide_all(login_window);
 
		 pthread_mutex_lock(&mutex); // 获取互斥锁
 
		 // 创建主窗口
		 create_linpopwnd();
		 gnum++;
		 if (gnum == 1)
			 pthread_cond_signal(&cond);
		 pthread_mutex_unlock(&mutex); // 释放互斥锁
 
		 if (access(my_id, 0) == -1) // 创建一个以自己 id 为名的文件夹用于保存聊天记录
		 {
			 if (mkdir(my_id, 0777))//如果不存在就用mkdir函数来创建
			 {
				 perror("creat folder failed");
			 }
		 }
		 return FALSE;
	 }
	 else if (utimer_count >= 50 && socket_ret == -1)
	 {
		 gdk_threads_enter();
		 show_info_msg_box(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				 "Connect server failed!");
		 gdk_threads_leave();
		 // 不这样加上就会死机
 
		 linpop_quit(NULL, NULL);
 
		 return FALSE;
	 }
 
	 return TRUE;
 }
 
 void set_user_info(GtkWidget *widget, gpointer data)
 {
	 GtkWidget *dialog = create_user_info_dlg();
 
	 GtkResponseType result = gtk_dialog_run(GTK_DIALOG (dialog));
 
	 if (result == GTK_RESPONSE_OK)
	 {
		 // 更新头像
		 gtk_container_remove(GTK_CONTAINER(button_image), image_user);
		 image_user_new = gtk_image_new_from_file(str_image_user_new);
		 image_user = image_user_new;
		 gtk_container_add(GTK_CONTAINER(button_image), image_user_new);
		 gtk_widget_show_all(button_image);
	 }
 
	 gtk_widget_destroy(dialog);
 }
 
 int show_info_msg_box(GtkMessageType msg_type, GtkButtonsType btn_type,gchar *text_error)
 {
	 GtkWidget *MSGBox = gtk_message_dialog_new(NULL,
			 GTK_DIALOG_DESTROY_WITH_PARENT, msg_type, btn_type, "%s",
			 text_error);
	 int ret = gtk_dialog_run(GTK_DIALOG(MSGBox));
	 gtk_widget_destroy(MSGBox);
	 return ret;
 }
 
 //背景
 void chang_background(GtkWidget *widget, int w, int h, const gchar *path)  
 {  
	 gtk_widget_set_app_paintable(widget, TRUE);     //允许窗口可以绘图  
	 gtk_widget_realize(widget);   
   
	 /* 更改背景图时，图片会重叠 
	  * 这时要手动调用下面的函数，让窗口绘图区域失效，产生窗口重绘制事件（即 expose 事件）。 
	  */  
	 gtk_widget_queue_draw(widget);  
   
	 GdkPixbuf *src_pixbuf = gdk_pixbuf_new_from_file(path, NULL);   // 创建图片资源对象  
	 // w, h是指定图片的宽度和高度  
	 GdkPixbuf *dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, w, h, GDK_INTERP_BILINEAR);  
   
	 GdkPixmap *pixmap = NULL;  
   
	 /* 创建pixmap图像;  
	  * NULL：不需要蒙版;  
	  * 123： 0~255，透明到不透明 
	  */  
	 gdk_pixbuf_render_pixmap_and_mask(dst_pixbuf, &pixmap, NULL, 128);  
	 // 通过pixmap给widget设置一张背景图，最后一个参数必须为: FASLE  
	 gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);  
   
	 // 释放资源  
	 g_object_unref(src_pixbuf);  
	 g_object_unref(dst_pixbuf);  
	 g_object_unref(pixmap);  
 }  
 
 GtkWidget *create_calendar(int argc,char *argv[ ])//添加日历窗口
 {
         GtkWidget *window;
	 GtkWidget *calendar;
  
		 gtk_init(&argc,&argv);
	 window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	 gtk_window_set_title (GTK_WINDOW (window), "日历");
	 gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	
	 gtk_window_set_resizable (GTK_WINDOW (window), FALSE);//窗口大小不可改变  
		  calendar=gtk_calendar_new();
	 gtk_calendar_display_options(GTK_CALENDAR(calendar),GTK_CALENDAR_SHOW_HEADING|GTK_CALENDAR_SHOW_DAY_NAMES|GTK_CALENDAR_SHOW_WEEK_NUMBERS|GTK_CALENDAR_WEEK_START_MONDAY);
	 gtk_calendar_select_month(GTK_CALENDAR(calendar), 8, 2017);
	 gtk_calendar_select_day(GTK_CALENDAR(calendar), 31);
	 gtk_container_add(GTK_CONTAINER(window),calendar);
  
	 gtk_widget_show(calendar);
	 gtk_widget_show(window);
		 return window;        
 }
 GtkWidget *create_game(int argc,char *argv[ ])//添加游戏选择窗口
 {
	GtkWidget *window;
  
	gtk_init(&argc,&argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "游戏");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	
	//gtk_window_set_resizable (GTK_WINDOW (window), FALSE);//窗口大小不可改变 
	gtk_widget_set_size_request(window, 150, 50); 

	GtkWidget *hbox_game;//存放按钮
	GtkWidget *button_greedy,*button_boom;
	
	hbox_game=gtk_hbox_new(FALSE,5);
	button_greedy=gtk_button_new_with_label("俄罗斯方块");
		set_widget_font_size(button_greedy, 10, TRUE);//设置大小
	button_boom  =gtk_button_new_with_label("扫 雷");
		set_widget_font_size(button_boom, 10, TRUE);//设置大小
	
	g_signal_connect(G_OBJECT(button_greedy),"clicked",G_CALLBACK(create_greedy),NULL);
	g_signal_connect(G_OBJECT(button_boom),"clicked",G_CALLBACK(swap_mine),NULL);
	
	gtk_box_pack_start(GTK_BOX(hbox_game), button_greedy, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox_game), button_boom, FALSE, FALSE, 5);

	gtk_container_add(GTK_CONTAINER(window), hbox_game);
	gtk_widget_show_all(window);
		 return window;        
 }
 void linpop_quit(GtkWidget *widget, gpointer data)
 {
	 gtk_timeout_remove(utimer_login);
	 thread_quit = TRUE;
	 gtk_main_quit();
 }

 /* 
 * File:   main.c
 * Author: ldd
 *
 * Created on 2012年1月8日, 下午12:38
 */

 
typedef struct ACTIVE_BLOCK_struct{
    int block[4][4];
    int left;//左面有多少个格
    int top;//上面有多少个格
    int height;
    int width;
}ACTIVE_BLOCK;
 
int _block[7][4][4]={
                     {{1,1,1,1},
                      {0,0,0,0},
                      {0,0,0,0},
                      {0,0,0,0}},//----
                     {{1,1,0,0},
                      {1,1,0,0},
                      {0,0,0,0},
                      {0,0,0,0}},//田
                     {{1,1,0,0},
                      {0,1,1,0},
                      {0,0,0,0},
                      {0,0,0,0}},//
                     {{0,1,1,0},
                      {1,1,0,0},
                      {0,0,0,0},
                      {0,0,0,0}},//
                     {{1,1,1,0},
                      {0,0,1,0},
                      {0,0,0,0},
                      {0,0,0,0}},//
                     {{1,1,1,0},
                      {1,0,0,0},
                      {0,0,0,0},
                      {0,0,0,0}},//
                     {{1,1,1,0},
                      {0,1,0,0},
                      {0,0,0,0},
                      {0,0,0,0}}
};//七种block
int width[7];//七种block的宽度
int height[7];//七种block的高度
ACTIVE_BLOCK block;//全局的动态 block
int map[32][16];//地图，最后一位代表有无块其他表示颜色
 
GtkWidget *button;
GtkWidget *table;
GtkWidget *label1;
GtkWidget *label2;
GtkWidget *draw_erea;
int score;//总分
int map_height;//map的总高度，用于消去时减少循环
int turn[4][4]={{30,20,10,0},
                {31,21,11,1},
                {32,22,12,2},
                {33,23,13,3}};//旋转数组
int a[4][4];
gboolean is_create;//是否生成新的block，即最后一个block无法下降了
int time_id;
GdkColor color;
GdkColor color_white;
GdkDrawable *draw_able;
GdkGC *gc;
GdkGC *gc_white;
GtkWidget *window;
GtkWidget *entry;
 
void init()//游戏初始化
{
    int i,j,k;
    score=0;
    map_height=0;
    is_create=TRUE;
    color.blue=0;
    color.red=65535;
    color.green=0;
    color_white.blue=65535;
    color_white.red=65535;
    color_white.green=65535;
    gc=gdk_gc_new(draw_erea->window);
    gc_white=gdk_gc_new(draw_erea->window);
    gdk_gc_set_rgb_fg_color(gc,&color);
    gdk_gc_set_rgb_fg_color(gc_white,&color_white);
    draw_able=draw_erea->window;
     
 
    height[0]=1;
    for(i=1;i<7;i++)
        height[i]=2;
    width[0]=4;
    width[1]=2;
    for(i=2;i<7;i++)
        width[i]=3;
    map[0][0]=1;
    map[0][1]=1;
}
 
void display_map()//显示map的block
{
    int i,j;
    gdk_draw_rectangle(draw_able,gc_white,TRUE,0,0,320,640);
    for(i=0;i<32;i++)
    {
        for(j=0;j<16;j++)
        {
            if(map[i][j]==1)
                gdk_draw_rectangle(draw_able,gc,TRUE,j*20,i*20,15,15);
        }
    }
}
 
void display_block()
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            if(block.block[i][j]%10==1)
                gdk_draw_rectangle(draw_able,gc,TRUE,(j+block.left)*20,(i+block.top)*20,15,15);
}
 
gboolean check_down(){//检查是否还可以下降
    int i,j,b;//b用来区分block是否在最后一列
    if(block.height+block.top>31)
    {
        return FALSE;
    }
    if(block.left==15&&block.width==1)
        b=1;
    else if(block.left==14&&block.width==2)
        b=2;
    else if(block.left==13&&block.width==3)
        b=3;
    else
        b=4;
    for(i=0;i<block.height;i++)
    {
        for(j=0;j<b;j++)
            if(block.block[i][j]+map[block.top+i+1][block.left+j]%10>1)
                return FALSE;
    }
    return TRUE;
}
 
gboolean check_now()//检查当前状态是否安全
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            if(a[i][j]+map[block.top+i][block.left+j]%10>1)
                return FALSE;
    return TRUE;
}
 
gboolean check_left_right()//检查左移或右移是否安全
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            if(block.block[i][j]+map[block.top+i][block.left+j]%10>1)
                return FALSE;
    return TRUE;
}
 
void un_display_block()
{
    int i,j;
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            if(block.block[i][j]%10==1)
                gdk_draw_rectangle(draw_able,gc_white,TRUE,(j+block.left)*20,(i+block.top)*20,15,15);
}
 
void eliminate(){//一行满了消去
    int i,j,k,temp;
    char a[20];
    for(i=31;i>=31-map_height+1;i--)
    {
        temp=0;
        for(j=0;j<16;j++)
            if(map[i][j]==0)
                break;
        if(j==16)
        {
            for(j=i;j>=31-map_height;j--)
                for(k=0;k<16;k++)
                    map[j][k]=map[j-1][k];
            i++;
            score++;
            gcvt((float)score,3,a);
            gtk_label_set_text(GTK_LABEL(label2),a);
        }
    }
}
 
void down()//下降函数
{
    int i,j,r;
    if(is_create==TRUE)//如果需要生成一个block
    {
        srandom(time(NULL));
        r=rand()%7;
        for(i=0;i<4;i++)
            for(j=0;j<4;j++)
                block.block[i][j]=_block[r][i][j];
        block.left=6;
        block.top=0;
        block.height=height[r];
        block.width=width[r];
        if(!check_down())///////////////////游戏结束
        {
            g_source_remove(time_id);
        }
        is_create=FALSE;
    }
    else
    {
        block.top++;
        if(!check_down())
        {
            for(i=0;i<block.height;i++)
                for(j=0;j<4;j++)
                    if(map[block.top+i][block.left+j]==0)
                        map[block.top+i][block.left+j]=block.block[i][j];
            if(32-block.top>map_height)
                map_height=32-block.top;
            eliminate();
            is_create=TRUE;
            display_map();
        }
    }
}
 
void timeout()
{
    un_display_block();
    down();
    display_map();
    display_block();
     
    g_source_remove(time_id);
    time_id=g_timeout_add(1000,(GtkFunction)timeout,NULL);
    return ;
}
 
void up()//按下上方向键，即变换形状，采用右旋转
{
    int i,j,temp,k;
  
    for(i=0;i<4;i++)
        for(j=0;j<4;j++)
            a[i][j]=block.block[turn[i][j]/10][turn[i][j]%10];
    if(check_now()==TRUE)
    {
        for(i=0;i<4;i++)
            for(j=0;j<4;j++)
                block.block[i][j]=a[i][j];
        temp=block.height;
        block.height=block.width;
        block.width=temp;
        for(i=0;i<3;i++)//左移
        {
            temp=a[0][i]+a[1][i]+a[2][i]+a[3][i];
            if(temp!=0)
                break;
            for(j=0;j<3;j++)
                for(k=0;k<4;k++)
                    block.block[k][j]=block.block[k][j+1];
            for(k=0;k<4;k++)
                    block.block[k][3]=0;
        }
    }
}
 
gboolean expose_event_callback(GtkWidget *widget,GdkEventExpose *event,gpointer data)
{
    display_map();
    display_block();
}
 
gboolean key_value(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    un_display_block();
    if(strcmp(gdk_keyval_name(event->keyval),"Up")==0)
    {
        up();
    }
    else if(strcmp(gdk_keyval_name(event->keyval),"Down")==0)
    {
        down();
        display_map();
    }
    else if(strcmp(gdk_keyval_name(event->keyval),"Left")==0)
    {
        if(block.left>0)
            block.left--;
        if(check_left_right()==FALSE)
            block.left++;
    }
    else if(strcmp(gdk_keyval_name(event->keyval),"Right")==0)
    {
        if(block.left+block.width-1<15)
            block.left++;
        if(check_left_right()==FALSE)
            block.left--;
    }
    display_block();
    return FALSE;
}
 
int create_greedy(int argc, char** argv) {
 
    GtkWidget *greedy_window; 
    gtk_init(&argc,&argv);
   
    greedy_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    entry = gtk_entry_new();
    table=gtk_table_new(2,2,FALSE);
    draw_erea=gtk_drawing_area_new();
    label1=gtk_label_new(g_locale_to_utf8("SCORE:",-1,NULL,NULL,NULL));
    label2=gtk_label_new(g_locale_to_utf8("0",-1,NULL,NULL,NULL));
    gtk_label_set_markup(GTK_LABEL(label2),"<span font_desc='40'>0</span>");
    gtk_widget_set_size_request(draw_erea,320,640);//设置大小
    gtk_container_set_border_width(GTK_CONTAINER(greedy_window),40);
    gtk_container_add(GTK_CONTAINER(greedy_window),table);
    gtk_table_attach(GTK_TABLE(table),draw_erea,0,1,0,2,
        (GtkAttachOptions)(0),(GtkAttachOptions)(0),20,20);
    gtk_table_attach(GTK_TABLE(table),label1,1,2,0,1,
        (GtkAttachOptions)(0),(GtkAttachOptions)(0),20,20);
    gtk_table_attach(GTK_TABLE(table),label2,1,2,1,2,
        (GtkAttachOptions)(0),(GtkAttachOptions)(0),20,20);
 
    g_signal_connect(G_OBJECT(draw_erea),"expose_event",G_CALLBACK(expose_event_callback),NULL);//重绘事件
    g_signal_connect(G_OBJECT(greedy_window),"key-press-event",G_CALLBACK(key_value),NULL);
     
    gtk_entry_new();
    time_id=g_timeout_add(1000,(GtkFunction)timeout,NULL);
    gtk_widget_show_all(greedy_window);
    init();
	gtk_main();
	g_signal_connect (G_OBJECT (greedy_window), "destroy",
	G_CALLBACK (gtk_main_quit), NULL);
	return (EXIT_SUCCESS);
}
//////////////////////////////////////////////////////////////扫雷//////////////////////////////////////////////////
struct block
{
    gint count; //表示一个点周围有多少个雷

    gboolean mine; //这个点是否藏有雷

    gboolean marked; //是否被标记有雷

    gboolean opened; //是否被掀开

    GtkWidget *button;
};
static struct block *dt; //整个地雷区图

static gint wid=10; //雷区的宽度

static gint hei=10; //雷区的高度

static gint mines=10; //总共的地雷数量

static GtkWidget *win_mine;
static GtkWidget *mine_label; //显示剩余的地雷数

static GtkWidget *time_label; //显示剩余的地雷时间

static GtkWidget *reset; //用于游戏结束后的重新复位

static gint button_size=25; //每个按钮的大小

static gint opened_count; //已掀开了多少个格子

static gint marked_count; //已经标记了多少个格子

static gboolean game_over; //游戏是否结束的标志

static gint game_time;//游戏时间的记录

static gint i=0, j,dexin;
static gint size;//整个地图区的大小

static gboolean mark;//用于标记是否按了复位键

static gboolean clicked;//用于表示是否按下掀开键

//游戏结束时调用的函数

void gameover(gboolean won)
  {
     GtkWidget *dialog;
        gchar buf[4];
     gchar msg[100];
        game_over=TRUE;
        if(won==TRUE)//扫雷成功的提示信息

          {
     g_snprintf(msg, 100, "哇！你这么牛，用时 %3d 秒。", game_time);
     }
        else//踩到地雷的情况

          {
     g_snprintf(msg, 100, "太有才了！这么快就玩完了～～");
     }
        for(i=0, dexin=0; i<hei; i++)//游戏结束后让所有格子都打开

          {
              for(j=0; j<wid; j++)
                {
                     if(dt[dexin].opened==FALSE)
                     {
                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt[dexin].button),TRUE);
                        gtk_button_set_label(GTK_BUTTON(dt[dexin].button), " ");
                         if(dt[dexin].mine==FALSE)
                          {
                             if(dt[dexin].count==0) gtk_button_set_label(GTK_BUTTON(dt[dexin].button), "");
                             else
                                {
                                  g_snprintf(buf, 4, "%d", dt[dexin].count);
                                  gtk_button_set_label(GTK_BUTTON(dt[dexin].button), buf);
                                 }
                           }
                        else gtk_button_set_label(GTK_BUTTON(dt[dexin].button), "*");
                       }
                  dexin++;
               }
         }
    //弹出窗口显示提示信息

    dialog=gtk_message_dialog_new(GTK_WINDOW(win_mine), 0,GTK_MESSAGE_INFO, GTK_BUTTONS_OK, msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
//布雷函数

void put_mines()
 {
   while(i<mines)
   {
        gint dexin;
    gint row, col;
    dexin=g_random_int_range(0, size);
    if(dt[dexin].mine==TRUE)//如果此点有雷则跳过

        continue;
    dt[dexin].mine=TRUE;
    row=dexin/wid;
    col=dexin%wid;

    //周围格子的count加1

    if(row>0)
           {
              if(col>0) dt[dexin-wid-1].count++;
              dt[dexin-wid].count++;
              if(col<wid-1) dt[dexin-wid+1].count++;
           }
         if(col>0) dt[dexin-1].count++;
         if(col<wid-1) dt[dexin+1].count++;
     if(row<hei-1)
           {
               if(col>0) dt[dexin+wid-1].count++;
               dt[dexin+wid].count++;
               if(col<wid-1) dt[dexin+wid+1].count++;
     }
          i++;
    }
 }
//打开某一个格子对应的函数

void open_block(gint x, gint y)
 {
     gint dexin;
     GtkWidget *button;
     dexin=x+y*wid;
     if(game_over==TRUE || dt[dexin].marked==TRUE)
        return; //游戏结束或防止某家对已标记的盒子打开

     button=dt[dexin].button;
     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);//改变状态为按下

     clicked=TRUE;//表示有敲击

     if(dt[dexin].opened==TRUE) //防止打开已经打开的盒子

        return;
     dt[dexin].opened=TRUE; //打开盒子

     if(dt[dexin].mine==TRUE)//如果此盒子下有雷

        {
     gtk_button_set_label(GTK_BUTTON(button), "*");
     gameover(FALSE); //踩到地雷游戏结束

     return;
    }
     if(dt[dexin].count>0)//若周围有雷

        {
     gchar buf[2];
     g_snprintf(buf, 2, "%d", dt[dexin].count);
     gtk_button_set_label(GTK_BUTTON(button), buf);
    }
       opened_count++; //增加一个已掀开的格子数

     if(opened_count+mines==wid*hei)//获胜的标志

        {
        gameover(TRUE);
        return;
    }
      if(dt[dexin].count==0)//若周围没有雷则掀开周围的格子

        {
     if(y>0)
             {
        if(x>0) open_block(x-1, y-1);
        open_block(x, y-1);
        if(x<wid-1) open_block(x+1, y-1);
     }
        if(x>0) open_block(x-1, y);
        if(x<wid-1) open_block(x+1, y);
        if(y<hei-1)
                   {
         if(x>0) open_block(x-1, y+1);
         open_block(x, y+1);
         if(x<wid-1) open_block(x+1, y+1);
         }
    }
 }
//复位

void g_reset(GtkWidget *widget,gpointer *data)
 {
    opened_count=0;
    marked_count=0;
    game_over=FALSE;
    mark=TRUE;
    clicked=FALSE;
    gtk_label_set_text(GTK_LABEL(mine_label), "10");
    for(i=0, dexin=0; i<hei; i++)
      {
         for(j=0; j<wid; j++)
          {
            dt[dexin].mine=FALSE;
            dt[dexin].count=0;
            dt[dexin].marked=FALSE;
            dt[dexin].opened=FALSE;
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dt[dexin].button),FALSE);
            gtk_button_set_label(GTK_BUTTON(dt[dexin].button), " ");
            dexin++;
           }
       }
   i=0;
   put_mines();
}
//计时器

gboolean tick(gpointer data)
 {
     gchar buf[8];
     if(game_over==FALSE)//计时

         {
           if(mark==TRUE) game_time=0,mark=FALSE;//得位标记mark为TRUE，则时间刷新为0

           if(clicked==TRUE)//如果有掀雷，则开始计时

             {
      game_time++;//增加游戏时间

      g_snprintf(buf, 8, "%d", game_time);
             }
           else g_snprintf(buf,2,"%d",game_time);//没有敲击时一直显示为0

           gtk_label_set_text(GTK_LABEL(time_label), buf);//刷新时间显示

          }
  return TRUE;//返回TRUE继续计时

 }
//键盘敲击时的敲击

gboolean on_mouse_click(GtkWidget *widget,GdkEventButton *event,gpointer data)
  {
     gint dexin;
     gint row, col;
     gchar buf[4];
     if(game_over==TRUE) return TRUE; //游戏结束时按键失去作用

     dexin=(gint)data;
     switch(event->button)
      {
    case 1://左键

        row=dexin/wid;
        col=dexin%wid;
        open_block(col, row);
        break;
    case 2: //中键

        break;
    case 3: //右键

        if(dt[dexin].opened==TRUE)//已掀开的不作记号

            break;
        //没有记号的加上，有的去掉

        if(dt[dexin].marked!=TRUE)
                   {
            dt[dexin].marked=TRUE;
            gtk_button_set_label(
            GTK_BUTTON(widget), "！");
            marked_count++;
         }
                else
                  {
            dt[dexin].marked=FALSE;
            gtk_button_set_label(
            GTK_BUTTON(widget), "");
            marked_count--;
         }
        //显示新的地雷数

        g_snprintf(buf, 4, "%d",mines-marked_count);
        gtk_label_set_text(GTK_LABEL(mine_label), buf);
    }
    return TRUE;
 }

gint swap_mine(gint argc,gchar **argv)
 {
   size=wid*hei;
   dt=(struct block *)g_malloc0(sizeof(struct block)*wid*hei);//分配内容为‘0’的的内存空间

   put_mines();

  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  gtk_init(&argc,&argv);
  win_mine=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(win_mine), "delete_event",gtk_main_quit, NULL);
  reset=gtk_button_new_with_label("重开");
  gtk_widget_set_usize(reset,100,28);
  vbox=gtk_vbox_new(FALSE, 0);
  hbox=gtk_hbox_new(FALSE, 0);
  label=gtk_label_new("Mines:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 6);
  mine_label=gtk_label_new("10");
  gtk_box_pack_start(GTK_BOX(hbox), mine_label,FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(hbox),reset,FALSE,TRUE,6);
  label=gtk_label_new("Time:");
  gtk_box_pack_start(GTK_BOX(hbox), label,FALSE, FALSE, 6);
  time_label=gtk_label_new("0");
  gtk_box_pack_start(GTK_BOX(hbox), time_label,TRUE, FALSE, 0);
  gtk_widget_show_all(hbox);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 0);
  for(i=0, dexin=0; i<hei; i++)//所有格子的初始化

     {
    gint j;
    hbox=gtk_hbox_new(FALSE, 0);
    for(j=0; j<wid; j++)
              {
        GtkWidget *button;
        button=gtk_toggle_button_new();
        gtk_widget_set_usize(button,
        button_size, button_size);
        g_object_set(G_OBJECT(button),"can-focus", FALSE, NULL);
        gtk_box_pack_start(GTK_BOX(hbox),button, FALSE, FALSE, 0);
        gtk_widget_show(button);
        g_signal_connect(G_OBJECT(button),"button-press-event",
        G_CALLBACK(on_mouse_click),(gpointer)dexin);
     dt[dexin].button=button;
        dexin++;
        }
        gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 0);
        gtk_widget_show(hbox);
    }
   gtk_container_add(GTK_CONTAINER(win_mine), vbox);
   gtk_widget_show(vbox);
   gtk_widget_show(win_mine);
   g_timeout_add(1000, (GSourceFunc)tick, NULL);//计时器启动

   g_signal_connect(GTK_OBJECT(reset),"clicked",GTK_SIGNAL_FUNC(g_reset),NULL);
  
    gtk_main();
    return 0;
}
