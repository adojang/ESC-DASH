# Specify the desired SSID and password
$desiredSSID = "acs"
$desiredPassword = "ASDF"

# Check if the hotspot is already enabled
$hotspotStatus = netsh wlan show hostednetwork | Select-String "Status" | ForEach-Object { $_ -match "Started" }

if ($hotspotStatus) {
    # Hotspot is already enabled
    $currentSSID = netsh wlan show hostednetwork | Select-String "SSID" | ForEach-Object { ($_ -split ":")[1].Trim() }

    if ($currentSSID -eq $desiredSSID) {
        # Hotspot is already enabled with the correct SSID, do nothing
        Write-Host "Hotspot is already enabled with the correct SSID ($desiredSSID)."
    }
    else {
        # Disable the current hotspot, change the SSID and password, and enable it again
        netsh wlan stop hostednetwork
        netsh wlan set hostednetwork mode=allow ssid="$desiredSSID" key="$desiredPassword"
        netsh wlan start hostednetwork
        Write-Host "Hotspot SSID changed to $desiredSSID and enabled."
    }
}
else {
    # Hotspot is not enabled, enable it with the specified SSID and password
    netsh wlan set hostednetwork mode=allow ssid="$desiredSSID" key="$desiredPassword"
    netsh wlan start hostednetwork
    Write-Host "Hotspot enabled with SSID $desiredSSID and password $desiredPassword."
}
