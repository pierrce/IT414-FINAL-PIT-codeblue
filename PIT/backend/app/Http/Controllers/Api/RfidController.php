<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Rfid;
use Illuminate\Http\Request;
use PhpMqtt\Client\Facades\MQTT;

class RfidController extends Controller
{
    // Get all RFID records (showing history)
    public function index()
    {
        $rfids = Rfid::orderBy('created_at', 'desc')->get();
        return response()->json($rfids);
    }

    public function store(Request $request)
    {
        $validated = $request->validate([
            'rfid_number' => 'required|string'
        ]);

        $rfidNumber = $validated['rfid_number'];

        // Check if RFID is REGISTERED (exists in base insert)
        $isRegistered = Rfid::where('rfid_number', $rfidNumber)
            ->where('registered', true)
            ->exists();

        $newStatus = 0; // Default: NOT FOUND

        if ($isRegistered) {
            // Toggle only for registered
            $latest = Rfid::where('rfid_number', $rfidNumber)
                ->orderBy('created_at', 'desc')
                ->first();
            $currentStatus = $latest ? $latest->status : 0;
            $newStatus = $currentStatus === 1 ? 0 : 1;
        }

        // LOG EVERY SCAN — BUT MARK registered CORRECTLY
        $log = Rfid::create([
            'rfid_number' => $rfidNumber,
            'status' => $newStatus,
            'registered' => $isRegistered  // ← CRITICAL
        ]);

        // ✅ PUBLISH TO MQTT (only status: 1 or 0)
        try {
            MQTT::publish('RFID_LOGIN', (string)$newStatus, 0, false);
        } catch (\Exception $e) {
            \Log::error('MQTT Publish Failed: ' . $e->getMessage());
        }

        return response()->json([
            'message' => $isRegistered ? 'Scan logged' : 'Unregistered scan logged',
            'rfid' => $log,
            'mqtt_status' => $newStatus  // Optional: show what was published
        ], 201);
    }

    // Update RFID status (from toggle button)
    public function update(Request $request, $id)
    {
        $rfid = Rfid::findOrFail($id);
        $rfid->status = $request->status;
        $rfid->save();

        // ✅ PUBLISH TO MQTT when manually toggled
        try {
            MQTT::publish('RFID_LOGIN', (string)$rfid->status, 0, false);
        } catch (\Exception $e) {
            \Log::error('MQTT Publish Failed: ' . $e->getMessage());
        }

        return response()->json($rfid);
    }
}