<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Rfid;
use Illuminate\Http\Request;

class RfidController extends Controller
{
    // Get all RFID records
    public function index()
    {
        $rfids = Rfid::orderBy('created_at', 'desc')->get();
        return response()->json($rfids);
    }

    // Store new RFID scan (from ESP32)
    public function store(Request $request)
    {
        $validated = $request->validate([
            'rfid_number' => 'required|string',
            'status' => 'required|integer|in:0,1'
        ]);

        $rfid = Rfid::create($validated);
        return response()->json($rfid, 201);
    }

    // Update RFID status (from toggle button)
    public function update(Request $request, $id)
    {
        $rfid = Rfid::findOrFail($id);
        $rfid->status = $request->status;
        $rfid->save();

        return response()->json($rfid);
    }

    // Delete RFID record
    public function destroy($id)
    {
        $rfid = Rfid::findOrFail($id);
        $rfid->delete();

        return response()->json(['message' => 'Deleted successfully']);
    }
}
