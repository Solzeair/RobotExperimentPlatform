# RobotExperimentPlatform

## 注意事项

- **特别注意** 每一次作业前请确认仓库是否有新的提交，若有新的提交请下载更新后再增加你的改动，最后确保无误后再提交，否则已经完成的工作会被你的提交覆盖，请务必仔细。
- 本仓库采用 *Github Flow* 工作流模型，每一次合并至 **main** 分支至少需要除了自己以外的一人同意后才可合并。
- 上传代码文件类型请参考 *.gitignore* 文件

## 配置环境

前提条件： 已经安装了 Qt，并且在 Visual Studio 中安装了 Qt VS Tools 扩展。

- 步骤 1：在 Visual Studio 中添加本机的 Qt 版本
	- 1. 在 Visual Studio 顶部菜单栏中，点击 扩展 (Extensions) -> Qt VS Tools -> Qt Versions。
	- 2. 在弹出的窗口中点击 添加 (Add new Qt version)（通常是一个加号图标）。
	- 3. 选择电脑上本地的 Qt 编译器路径（例如：C:\Qt\5.15.2\msvc2019_64）。
	- 4. 确认并保存。
- 步骤 2：为该项目分配刚刚配置的 Qt 版本
	- 1. 在 解决方案资源管理器 (Solution Explorer) 中，右键点击项目名称 QtWidgetDesign。
	- 2. 选择 属性 (Properties)。
	- 3. 在左侧菜单中找到 Qt Project Settings。
	- 4. 在右侧的 Qt Installation 选项中，点击下拉菜单，选择在“步骤 1”中配置好的那个 Qt 版本。
	- 5. 点击 确定 保存。
- 步骤 3：重新生成
	- 1. 右键点击项目，选择 清理 (Clean)。
	- 2. 然后选择 重新生成 (Rebuild)。