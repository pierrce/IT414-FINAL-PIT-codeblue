<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;
use Illuminate\Support\Facades\DB;

class CreateRfidsTable extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('rfids', function (Blueprint $table) {
            $table->id();
            $table->string('rfid_number');
            $table->integer('status')->default(0); // 0 = NOT FOUND, 1 = FOUND
            $table->boolean('registered')->default(false); // ← ADDED
            $table->timestamps();
        });

        // INSERT YOUR 2 REGISTERED RFIDs
        DB::table('rfids')->insert([
            [
                'rfid_number' => '22C19E7A',
                'status' => 0,
                'registered' => true, // ← MARK AS REGISTERED
                'created_at' => now(),
                'updated_at' => now(),
            ],
            [
                'rfid_number' => 'FFC6AADE',
                'status' => 0,
                'registered' => true, // ← MARK AS REGISTERED
                'created_at' => now(),
                'updated_at' => now(),
            ],
        ]);
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('rfids');
    }
}
    