<?php
// database/migrations/xxxx_xx_xx_xxxxxx_create_rfids_table.php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

class CreateRfidsTable extends Migration
{
    public function up()
    {
        Schema::create('rfids', function (Blueprint $table) {
            $table->id();
            $table->string('rfid_number')->unique(); // Make sure this is unique
            $table->integer('status')->default(1); // 0 or 1
            $table->timestamps();
        });
    }

    public function down()
    {
        Schema::dropIfExists('rfids');
    }
}