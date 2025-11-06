<script>
  import { onMount } from 'svelte';

  let rfidData = [];
  let loading = true;
  let error = null;

  // Fetch RFID data from Laravel API
  async function fetchRfids() {
    try {
      loading = true;
      const response = await fetch('http://127.0.0.1:8000/api/rfids');
      if (!response.ok) throw new Error('Failed to fetch data');
      rfidData = await response.json();
      loading = false;
    } catch (err) {
      error = err.message;
      loading = false;
    }
  }

  // Toggle RFID status
  async function toggleStatus(id) {
    const item = rfidData.find(r => r.id === id);
    const newStatus = item.status === 1 ? 0 : 1;

    try {
      const response = await fetch(`http://127.0.0.1:8000/api/rfids/${id}`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ status: newStatus })
      });

      if (response.ok) {
        // Update local data
        rfidData = rfidData.map(r => 
          r.id === id ? { ...r, status: newStatus } : r
        );
      }
    } catch (err) {
      console.error('Error updating status:', err);
    }
  }

  // Format timestamp
  function formatDate(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleString('en-US', {
      month: 'long',
      day: 'numeric',
      year: 'numeric',
      hour: 'numeric',
      minute: '2-digit',
      hour12: true
    });
  }

  // Fetch data when component loads
  onMount(() => {
    fetchRfids();
    // Auto-refresh every 5 seconds
    const interval = setInterval(fetchRfids, 5000);
    return () => clearInterval(interval);
  });
</script>

<main>
  <div class="container">
    <!-- Checkbox section on the RIGHT -->
    <div class="checkbox-section">
      <h3>RFID</h3>
      {#if loading}
        <p style="color: #666;">Loading...</p>
      {:else if error}
        <p style="color: red;">{error}</p>
      {:else}
        {#each rfidData.slice(0, 3) as item}
          <div class="checkbox-item">
            <span>{item.id}. {item.rfid_number}</span>
            <input
              type="checkbox"
              id="toggle-{item.id}"
              checked={item.status === 1}
              on:change={() => toggleStatus(item.id)}
            />
            <label for="toggle-{item.id}"></label>
          </div>
        {/each}
      {/if}
    </div>

    <!-- Table on the LEFT -->
    <table>
      <thead>
        <tr>
          <th>RFID</th>
          <th>Status</th>
          <th>Date & Time</th>
        </tr>
      </thead>
      <tbody>
        {#if loading}
          <tr><td colspan="3" style="text-align: center;">Loading...</td></tr>
        {:else if error}
          <tr><td colspan="3" style="text-align: center; color: red;">Error: {error}</td></tr>
        {:else if rfidData.length === 0}
          <tr><td colspan="3" style="text-align: center;">No data available</td></tr>
        {:else}
          {#each rfidData as item, index}
            <tr>
              <td>{index + 1}. {item.rfid_number}</td>
              <td>
                {#if item.status === 1}
                  1
                {:else}
                  RFID NOT FOUND
                {/if}
              </td>
              <td>{formatDate(item.created_at)}</td>
            </tr>
          {/each}
        {/if}
      </tbody>
    </table>
  </div>
</main>

<style>
  main {
    padding: 20px;
    font-family: Arial, sans-serif;
    color: #f2f2f2;
  }

  .container {
    display: flex;
    gap: 20px;
    max-width: 1400px;
  }

  .checkbox-section {
    min-width: 200px;
    border: 2px solid #000000;
    padding: 15px;
    background: rgb(254, 254, 254);
  }

  .checkbox-section h3 {
    margin: 0 0 15px 0;
    font-size: 16px;
    color: #000000;
    font-weight: bold;
  }

  .checkbox-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 10px;
    font-size: 14px;
    color: #000000;
  }

  /* Hide the default checkbox */
  .checkbox-item input[type="checkbox"] {
    display: none;
  }

  /* Toggle switch container */
  .checkbox-item label {
    position: relative;
    display: inline-block;
    width: 44px;
    height: 24px;
    cursor: pointer;
  }

  /* Toggle switch background */
  .checkbox-item label::before {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    width: 44px;
    height: 24px;
    background-color: #ccc;
    border-radius: 24px;
    transition: background-color 0.3s;
  }

  /* Toggle switch circle/slider */
  .checkbox-item label::after {
    content: '';
    position: absolute;
    top: 2px;
    left: 2px;
    width: 20px;
    height: 20px;
    background-color: rgb(255, 255, 255);
    border-radius: 50%;
    transition: transform 0.3s;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
  }

  /* When checkbox is checked - change background to blue */
  .checkbox-item input[type="checkbox"]:checked + label::before {
    background-color: #3932ff;
  }

  /* When checkbox is checked - move slider to the right */
  .checkbox-item input[type="checkbox"]:checked + label::after {
    transform: translateX(20px);
  }

  table {
    flex: 1;
    width: 100%;
    border-collapse: collapse;
    border: 2px solid #000;
    background: rgb(235, 233, 233);
    color: #000000;
  }

  th, td {
    border: 1px solid #000000;
    padding: 10px;
    text-align: left;
    color: #000000;
  }

  th {
    background: #7d7c7c;
    font-weight: bold;
    color: #ffffff;
  }

  tbody tr:nth-child(even) {
    background: #ffffff;
  }

  /* Mobile Responsive */
  @media (max-width: 768px) {
    .container {
      flex-direction: column;
    }

    table, th, td {
      font-size: 12px;
    }

    th, td {
      padding: 8px;
    }
  }
</style>