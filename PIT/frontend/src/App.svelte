<script>
  import { onMount } from "svelte";

  let activeRfids = [];   // ← Only registered RFIDs
  let logs = [];          // ← Full scan history
  let loading = true;
  let error = null;
  let isOnline = true;

  const API_URL = "http://10.16.42.197:8000/api/rfids";

  // Fetch and separate data
  async function fetchRfids() {
    try {
      loading = true;
      const response = await fetch(API_URL);
      if (!response.ok) throw new Error('Failed to fetch data');

      const allData = await response.json();

      const registeredGroup = {};
      allData.forEach(item => {
        if (item.registered) { // ← FILTER HERE
          const current = registeredGroup[item.rfid_number];
          const itemTime = new Date(item.created_at).getTime();
          if (!current || itemTime > new Date(current.created_at).getTime()) {
            registeredGroup[item.rfid_number] = item;
          }
        }
      });

      // 2. activeRfids = only registered
      activeRfids = Object.values(registeredGroup)
        .sort((a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime())
        .slice(0, 5);

      // 3. logs = ALL scans (including unregistered)
      logs = allData.sort((a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime())

      // Save to localStorage
      localStorage.setItem('activeRfids', JSON.stringify(activeRfids));
      localStorage.setItem('logs', JSON.stringify(logs));
      localStorage.setItem('lastFetch', new Date().toISOString());

      isOnline = true;
      loading = false;
      error = null;
    } catch (err) {
      console.error('Fetch error:', err);

      const cachedActive = localStorage.getItem('activeRfids');
      const cachedLogs = localStorage.getItem('logs');
      if (cachedActive && cachedLogs) {
        activeRfids = JSON.parse(cachedActive);
        logs = JSON.parse(cachedLogs);
        error = 'Offline: ' + formatDate(localStorage.getItem('lastFetch'));
      } else {
        error = 'No connection & no cache';
        activeRfids = [];
        logs = [];
      }
      loading = false;
    }
  }

  // Toggle only affects the LATEST log for that RFID
  async function toggleStatus(rfidNumber) {
    const latestLog = logs.find(l => l.rfid_number === rfidNumber);
    if (!latestLog) return;

    const newStatus = latestLog.status === 1 ? 0 : 1;

    // Optimistically update
    logs = logs.map(l => 
      l.rfid_number === rfidNumber && l.id === latestLog.id 
        ? { ...l, status: newStatus } 
        : l
    );

    try {
      const response = await fetch(`${API_URL}/${latestLog.id}`, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ status: newStatus })
      });

      if (!response.ok) throw new Error('Update failed');
      await fetchRfids(); // Refresh both sections
    } catch (err) {
      console.error(err);
      alert('Failed to update. Reverting...');
      await fetchRfids();
    }
  }

  function formatDate(dateString) {
    if (!dateString) return 'N/A';
    const date = new Date(dateString);
    return date.toLocaleString("en-US", {
      month: "long", day: "numeric", year: "numeric",
      hour: "numeric", minute: "2-digit", hour12: true
    });
  }

  onMount(() => {
    fetchRfids();
    const interval = setInterval(fetchRfids, 5000);
    return () => clearInterval(interval);
  });
</script>

<main>
  <header class="dashboard-header">
    <h1>
      <span class="codeblue">CodeBlue</span> RFID Monitoring Dashboard
    </h1>
  </header>

  <div class="container">
    <!-- LEFT PANEL -->
<section class="left-panel">
  <div class="card">
    <h3>Registered RFID</h3>
    {#if loading && activeRfids.length === 0}
      <p class="loading-text">Loading...</p>
    {:else if error && activeRfids.length === 0}
      <p class="error">{error}</p>
    {:else}
      {#each activeRfids as item, index}
        <div class="rfid-row">
          <span class="index">{index + 1}.</span>
          <span class="rfid-num">{item.rfid_number}</span>
          <div class="switch disabled">
           <input
             type="checkbox"
             checked={item.status === 1}
             disabled/>
            <span class="slider"></span>
          </div>
        </div>
      {/each}
    {/if}
  </div>
</section>

    <!-- RIGHT PANEL -->
<section class="right-panel">
  <div class="card">
    <div class="card-header">
      <h3>RFID Logs</h3>
    </div>
    <div class="table-wrapper">
      <table>
        <thead>
          <tr>
            <th>#</th>
            <th>RFID</th>
            <th>Status</th>
            <th>Date &amp; Time</th>
          </tr>
        </thead>
        <tbody>
          {#if loading && logs.length === 0}
            <tr><td colspan="4" class="center">Loading RFID data...</td></tr>
          {:else if logs.length === 0}
            <tr><td colspan="4" class="center">No RFID records found</td></tr>
          {:else}
            {#each logs as item, i}
              <tr>
                <td>{i + 1}</td>
                <td class="rfid-cell">{item.rfid_number}</td>
             <td>
  {#if item.registered}
    <span class:found={item.status === 1} class:notfound={item.status === 0}>
      {item.status}
    </span>
  {:else}
    <span class="notfound">RFID NOT FOUND</span>
  {/if}
</td>
                <td class="nowrap">{formatDate(item.created_at)}</td>
              </tr>
            {/each}
          {/if}
        </tbody>
      </table>
    </div>
  </div>
</section>
  </div>
</main>

<style>
  :global(body) {
    margin: 0;
    font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
    background: #f0f4f8;
    color: #1e293b;
  }

  main {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 25px;
    min-height: 100vh;
  }

  /* HEADER */
  .dashboard-header {
    text-align: center;
    margin-bottom: 20px;
    position: relative;
  }

  .dashboard-header h1 {
    font-size: 1.9rem;
    font-weight: 600;
    color: #1e293b;
    margin: 0;
  }

  .codeblue {
    color: #2563eb;
    font-weight: 800;
    letter-spacing: 1px;
    font-size: 2.2rem;
    text-shadow: 1px 1px 2px rgba(37, 99, 235, 0.2);
  }

  /* CONTAINER */
  .container {
    display: flex;
    width: 90%;
    max-width: 1400px;
    gap: 25px;
  }

  /* CARD */
  .card {
    background: #ffffff;
    border-radius: 12px;
    box-shadow: 0 4px 16px rgba(0, 0, 0, 0.08);
    padding: 24px;
  }

  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 16px;
  }


  /* LEFT PANEL */
  .left-panel {
    flex: 0 0 340px;
  }

  .left-panel h3 {
    font-size: 1.15rem;
    margin-bottom: 16px;
    color: #2563eb;
    font-weight: 600;
  }

  .rfid-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    background: #f8fafc;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    margin-bottom: 10px;
    padding: 12px 16px;
    transition: all 0.2s ease;
  }

  .rfid-row:hover {
    background: #eff6ff;
    border-color: #bfdbfe;
    transform: translateY(-1px);
  }

  .rfid-num {
    flex: 1;
    margin-left: 10px;
    font-weight: 500;
    font-size: 0.95rem;
    font-family: 'Courier New', monospace;
  }

  .index {
    font-weight: 700;
    color: #2563eb;
    min-width: 25px;
  }

  .loading-text {
    text-align: center;
    color: #64748b;
    padding: 20px;
  }

  .error {
    color: #dc2626;
    padding: 12px;
    background: #fee2e2;
    border-radius: 6px;
    font-size: 0.9rem;
  }

  /* SWITCH */
  .switch {
    position: relative;
    display: inline-block;
    width: 46px;
    height: 26px;
  }

  .switch input {
    opacity: 0;
    width: 0;
    height: 0;
  }

  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #cbd5e1;
    transition: 0.3s;
    border-radius: 26px;
  }

  .slider:before {
    position: absolute;
    content: "";
    height: 20px;
    width: 20px;
    left: 3px;
    bottom: 3px;
    background-color: white;
    transition: 0.3s;
    border-radius: 50%;
    box-shadow: 0 2px 4px rgba(0,0,0,0.2);
  }

  input:checked + .slider {
    background-color: #22c55e;
  }

  input:checked + .slider:before {
    transform: translateX(20px);
  }

  /* RIGHT PANEL */
  .right-panel {
    flex: 1;
  }

  .right-panel h3 {
    font-size: 1.15rem;
    margin: 0;
    color: #2563eb;
    font-weight: 600;
  }

  .table-wrapper {
    width: 100%;
    overflow-x: auto;
  }

  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 0.9rem;
  }

  th, td {
    padding: 14px 12px;
    text-align: left;
  }

  th {
    background: #2563eb;
    color: white;
    font-weight: 600;
    white-space: nowrap;
  }

  th:first-child {
    border-top-left-radius: 8px;
  }

  th:last-child {
    border-top-right-radius: 8px;
  }

  td {
    border-bottom: 1px solid #e2e8f0;
  }

  tbody tr:hover {
    background: #f8fafc;
  }

  tbody tr:last-child td {
    border-bottom: none;
  }

  .rfid-cell {
    font-family: 'Courier New', monospace;
    font-weight: 600;
    color: #1e293b;
  }

  .nowrap {
    white-space: nowrap;
  }

  .found {
  color: #16a34a; /* Green */
  font-weight: 600;
}

.notfound {
  color: #dc2626; /* Red */
  font-weight: 600;
}

  .center {
    text-align: center;
    color: #94a3b8;
    font-style: italic;
  }

  /* RESPONSIVE */
  @media (max-width: 900px) {
    .container {
      flex-direction: column;
      width: 95%;
      gap: 20px;
    }

    .left-panel {
      width: 100%;
    }

    table {
      font-size: 0.85rem;
    }

    th, td {
      padding: 10px 8px;
    }
  }

  @media (max-width: 600px) {
    .dashboard-header h1 {
      font-size: 1.5rem;
    }

    .codeblue {
      font-size: 1.8rem;
    }

    .container {
      width: 100%;
      padding: 0 10px;
    }

    .card {
      padding: 16px;
    }

    .card-header {
      flex-direction: column;
      align-items: flex-start;
      gap: 10px;
    }
    .switch.disabled {
  pointer-events: none;
  opacity: 0.85;
}

.switch.disabled input:checked + .slider {
  background-color: #22c55e;
}

.switch.disabled .slider {
  background-color: #cbd5e1;
}
  }
</style>