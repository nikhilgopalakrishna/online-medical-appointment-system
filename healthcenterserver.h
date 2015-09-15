typedef struct users_info{
    char userinfo[100];
}usersinfo_t;

typedef struct availability_info{
    char  availabily_info[100];
    char doctor_port[100];
    int reserved;
} availabilityinfo_t;

typedef struct server_handler_args{
 int  client_port ;
 int  new_fd;
 char s[INET6_ADDRSTRLEN];
}server_handler_args_t;
