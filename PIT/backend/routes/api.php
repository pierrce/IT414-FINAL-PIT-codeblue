<?php

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\RfidController;

Route::get('/rfids', [RfidController::class, 'index']);
Route::post('/rfids', [RfidController::class, 'store']);
Route::put('/rfids/{id}', [RfidController::class, 'update']);
Route::delete('/rfids/{id}', [RfidController::class, 'destroy']);
