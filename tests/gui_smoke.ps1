# Uses a temporary data folder: never touches the user's normal student database.
$ErrorActionPreference = 'Stop'
Add-Type -TypeDefinition @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class CwaUI {
    public delegate bool Callback(IntPtr h, IntPtr p);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h, Callback c, IntPtr p);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll", CharSet=CharSet.Unicode, EntryPoint="SendMessageW")] public static extern IntPtr Text(IntPtr h, uint m, IntPtr w, string s);
    [DllImport("user32.dll", EntryPoint="SendMessageW")] public static extern IntPtr Send(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] public static extern int GetDlgCtrlID(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out Rect r);
    public struct Rect { public int Left, Top, Right, Bottom; }
}
'@
function Children {
    $items = [System.Collections.Generic.List[object]]::new()
    $callback = [CwaUI+Callback]{param($h,$p)
        $t=[Text.StringBuilder]::new(1024);$c=[Text.StringBuilder]::new(128)
        [void][CwaUI]::GetWindowText($h,$t,1024);[void][CwaUI]::GetClassName($h,$c,128)
        $items.Add([pscustomobject]@{Handle=$h;Text=$t.ToString();Class=$c.ToString()})
        return $true
    }
    [void][CwaUI]::EnumChildWindows($script:window,$callback,[IntPtr]::Zero)
    return $items.ToArray()
}
function Click-Button($caption) {
    $button=@(Children | Where-Object {$_.Class -eq 'Button' -and $_.Text -eq $caption})
    if($button.Count -ne 1){throw "Button not found: $caption"}
    # Send the real button click so the app receives its normal BN_CLICKED command.
    [void][CwaUI]::Send($button[0].Handle,0xF5,[IntPtr]::Zero,[IntPtr]::Zero)
}
function Set-Edit($index,$value) {
    $edits=@(Children | Where-Object {$_.Class -eq 'Edit'})
    if($index -ge $edits.Count){throw "Edit field missing: $index"}
    [void][CwaUI]::Text($edits[$index].Handle,0xC,[IntPtr]::Zero,$value)
}
function Assert-UI($condition,$message){if(-not $condition){throw $message}}
function Start-App {
    $script:app=Start-Process -FilePath $script:exe -ArgumentList @('--data-dir',('"'+$script:data+'"')) -PassThru
    $deadline=[DateTime]::UtcNow.AddSeconds(20)
    do {
        Start-Sleep -Milliseconds 200;$script:app.Refresh()
        if($script:app.HasExited){throw "Application exited early: $($script:app.ExitCode)"}
        $script:window=$script:app.MainWindowHandle
    } until($script:window -ne [IntPtr]::Zero -or [DateTime]::UtcNow -gt $deadline)
    Assert-UI ($script:window -ne [IntPtr]::Zero) 'Application window not created.'
    [void]$script:app.WaitForInputIdle(10000)
}
function Close-App {
    [void][CwaUI]::Send($script:window,0x10,[IntPtr]::Zero,[IntPtr]::Zero)
    if(-not $script:app.WaitForExit(10000)){throw 'Application did not close cleanly.'}
    Assert-UI ($script:app.ExitCode -eq 0) 'Application exited with an error.'
}
$root=Split-Path -Parent $PSScriptRoot
$script:exe=Join-Path $root 'bin/Release/UMaT_CWA_Studio.exe'
$script:data=Join-Path ([IO.Path]::GetTempPath()) ('cwa-gui-'+[guid]::NewGuid())
try {
    Start-App
    Click-Button '+ New student'
    Set-Edit 0 'TEST001';Set-Edit 1 'Preview Student'
    Click-Button 'Continue to courses  >'
    Assert-UI (@(Children | Where-Object {$_.Text -eq 'COURSE CODE'}).Count -eq 1) 'Continue did not replace the student screen.'
    $codes=@('EL201','MA201','EP201');$marks=@('80','70','90');$credits=@('3','3','2')
    for($i=0;$i -lt 3;$i++){
        Set-Edit 0 $codes[$i];Set-Edit 1 ('Sample '+$codes[$i]);Set-Edit 2 'S1';Set-Edit 3 $marks[$i];Set-Edit 4 $credits[$i]
        Click-Button 'Add course +'
        $edits=@(Children | Where-Object {$_.Class -eq 'Edit'})
        Assert-UI (($edits[0].Text -eq '') -and ($edits[1].Text -eq '') -and ($edits[3].Text -eq '') -and ($edits[4].Text -eq '')) 'Course fields were not cleared.'
    }
    Click-Button 'Save & View Database'
    $controls=@(Children)
    Assert-UI (@($controls | Where-Object {$_.Class -eq 'SysListView32'}).Count -eq 2) 'Saved results tables missing.'
    Assert-UI (@($controls | Where-Object {$_.Text -like '*78.75%*'}).Count -gt 0) 'Expected saved average 78.75% missing.'
    Assert-UI (@($controls | Where-Object {$_.Text -eq 'COURSE CODE'}).Count -eq 0) 'Entry screen was not replaced.'
    Assert-UI (Test-Path (Join-Path $script:data 'umat_cwa.db')) 'Database was not created.'
    New-Item -ItemType Directory -Force (Join-Path $root 'build') | Out-Null
    try {
        Add-Type -AssemblyName System.Drawing
        $r=New-Object CwaUI+Rect;[void][CwaUI]::GetWindowRect($script:window,[ref]$r)
        $bitmap=[Drawing.Bitmap]::new($r.Right-$r.Left,$r.Bottom-$r.Top)
        $graphics=[Drawing.Graphics]::FromImage($bitmap)
        $graphics.CopyFromScreen($r.Left,$r.Top,0,0,$bitmap.Size)
        $bitmap.Save((Join-Path $root 'build/windows-results.png'))
        $graphics.Dispose();$bitmap.Dispose()
    } catch {Write-Warning "Screenshot unavailable: $_"}
    Close-App
    Start-App
    Assert-UI (@(Children | Where-Object {$_.Text -like '*78.75%*'}).Count -gt 0) 'Saved results did not survive restart.'
    Close-App
    Write-Host 'GUI smoke passed: form replacement, course clearing, database view, average and restart persistence.'
} finally {
    if($script:app -and -not $script:app.HasExited){Stop-Process -Id $script:app.Id -Force}
    if(Test-Path $script:data){Remove-Item -Recurse -Force $script:data}
}
