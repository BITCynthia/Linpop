/***********************************************************************
*                         登录                                          *
*                            主界面                                      *
*                               聊天                                    *
*                                  ……                                   *
*                                    连接                               *
************************************************************************/

#ifndef LINPOPWND_H_
#define LINPOPWND_H_

int gtk_wnd_init(int argc, char *argv[]);//同步所用，主窗口创建后再收到用户在线数据
GtkWidget *create_login_wnd();//创建登录主窗口
GtkWidget *create_linpopwnd();//登录成功窗口
void enter_callback(GtkWidget *widget, GtkWidget *entry);//响应回车函数
gboolean progress_login_timeout(GtkProgressBar *progress);//连接服务器时长检测
void login(GtkWidget *widget, gpointer data);//id/password检测是否可登录
void set_user_info(GtkWidget *widget, gpointer data);//更改用户信息
int show_info_msg_box(GtkMessageType msg_type, GtkButtonsType btn_type,
		gchar *text_error);//提示信息
void chang_background(GtkWidget *widget, int w, int h, const gchar *path) ;//背景
GtkWidget *create_calendar(int argc,char *argv[ ]);//日历
GtkWidget *create_game(int argc,char *argv[ ]);//添加游戏选择窗口
int create_greedy(int argc, char** argv) ;
gint swap_mine(gint argc,gchar **argv);
void linpop_quit(GtkWidget *widget, gpointer data);//回调_退出

#endif /* LINPOPWND_H_ */
