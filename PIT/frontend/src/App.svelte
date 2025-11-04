<script>
  // Dummy RFID data matching the reference
  let rfidData = [
    { id: 1, rfid: '88697684', status: 1, timestamp: 'October 10, 2025 2:00PM' },
    { id: 2, rfid: '09780647', status: 1, timestamp: 'October 10, 2025 1:00PM' },
    { id: 3, rfid: '75834600', status: 0, timestamp: 'October 10, 2025 1:00PM' },
    { id: 4, rfid: '90875490', status: 0, timestamp: 'October 10, 2025 1:00PM' },
    { id: 5, rfid: '88697684', status: 1, timestamp: 'October 10, 2025 1:00PM' },
    { id: 6, rfid: '88697684', status: 0, timestamp: 'October 10, 2025 1:00PM' },
    { id: 7, rfid: '90875490', status: 0, timestamp: 'imo mama, 2025 1:00PM' },
    { id: 8, rfid: '75834600', status: 0, timestamp: 'October 10, 2025 1:00PM' },
    { id: 9, rfid: '90875490', status: 0, timestamp: 'October 10, 2025 1:00PM' },
    { id: 10, rfid: '75834600', status: 1, timestamp: 'October 10, 2025 1:00PM' },
  ];

  // Function to toggle status
  function toggleStatus(id) {
    rfidData = rfidData.map(item => {
      if (item.id === id) {
        return { ...item, status: item.status === 1 ? 0 : 1 };
      }
      return item;
    });
  }
</script>

<main>
  <div class="container">
    <!-- Checkbox section on the RIGHT -->
    <div class="checkbox-section">
      <h3>RFID</h3>
      {#each rfidData.slice(0, 3) as item}
        <div class="checkbox-item">
          <span>{item.id}. {item.rfid}</span>
          <input
            type="checkbox"
            id="toggle-{item.id}"
            checked={item.status === 1}
            on:change={() => toggleStatus(item.id)}
          />
          <label for="toggle-{item.id}"></label>
        </div>
      {/each}
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
        {#each rfidData as item, index}
          <tr>
            <td>{index + 1}. {item.rfid}</td>
            <td>
              {#if item.status === 1}
                1
              {:else}
                RFID NOT FOUND
              {/if}
            </td>
            <td>{item.timestamp}</td>
          </tr>
        {/each}
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
    color: #000000;  /* ← Changed to black so it's visible! */
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

  /* When checkbox is checked - change background to red */
  .checkbox-item input[type="checkbox"]:checked + label::before {
    background-color: #3932ff;  /* Red color */
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