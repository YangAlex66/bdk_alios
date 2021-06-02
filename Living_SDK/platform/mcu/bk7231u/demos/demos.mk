NAME := demos

$(NAME)_INCLUDES := ../beken/func/video_transfer \
                    ../beken/ip/mac \
                    ../beken/func/user_driver \
					
$(NAME)_INCLUDES += ./ \
                    ./application/video_transfer
					

$(NAME)_SOURCES :=  ./demos_start.c \
                    ./application/video_transfer/app_demo_softap.c \
                    ./application/video_transfer/app_demo_tcp.c \
                    ./application/video_transfer/app_demo_udp.c \
