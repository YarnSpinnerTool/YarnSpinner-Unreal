# Yarn Spinner for Unreal

Welcome to **Yarn Spinner for Unreal!** Yarn Spinner is the friendly dialogue tool that makes it easy for writers to create content, and has powerful features for programmers.

> **Warning**
> This plugin is in an **extremely early alpha** state.

## Installation Instructions

1. If your project is already open in the Unreal Editor, close it.
2. Create a new folder called `Plugins` in the root of your project. (If you already have this folder, skip this step.)
3. Next, clone this repository and put it in the `Plugins` folder.
4. Finally, re-open your project. When Unreal Editor asks if you want to build the Yarn Spinner module, say yes.

## Troubleshooting

### I get a "Plugin 'YarnSpinner' failed to load because module 'YarnSpinner' could not be found.

If your project was created as a Blueprint Project, then Unreal will not include plugins by default. Add a new empty C++ class (of any type), and rebuild your project.
