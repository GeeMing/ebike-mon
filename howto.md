
更新组件
将组件管理器描述文件 idf_component.yml 中的组件版本号修改为要更新到的目标版本

删除工程文件中的 managed_components 和 dependencies.lock

执行 idf.py build 或手动执行 idf.py reconfigure