from django.urls import path
from . import views
urlpatterns = [
    path('system/', views.system_manage_page, name='system_manage'),
    path('api/get_rooms_json/', views.get_rooms_json, name='get_rooms_json'),

    path('api/sync_user/', views.sync_user, name='sync_user'),
    path('api/delete_user/', views.sync_delete_user, name='delete_user'),  # <--- 新增
    path('api/backup/', views.backup_data, name='backup_data'),
    path('api/restore/', views.restore_data, name='restore_data'),

    path('dashboard/', views.system_manage_page),
]