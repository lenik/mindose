# Applications subdirectories
subdir('notepad')
subdir('paint')
subdir('minesweeper')
subdir('calculator')
subdir('filemanager')
subdir('clock')
subdir('controlpanel')
subdir('terminal')

# Collect all app executables for convenience
app_executables = {
  'notepad' : notepad_app,
  'paint' : paint_app,
  'minesweeper' : minesweeper_app,
  'calculator' : calculator_app,
  'filemanager' : filemanager_app,
  'clock' : clock_app,
  'controlpanel' : controlpanel_app,
  'terminal' : terminal_app
}

# Test targets for individual applications
foreach app_name, app_exe : app_executables
  run_target('test-' + app_name,
    command : app_exe
  )
endforeach
