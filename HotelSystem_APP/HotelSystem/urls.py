from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),

    # 原有的路径
    path('smart_hotel/', include('smart_hotel.urls')),

    # 【新增】将空路径也指向 smart_hotel 的路由配置
    # 这样访问 http://127.0.0.1:8000/ 就会进入系统
    path('', include('smart_hotel.urls')),
]