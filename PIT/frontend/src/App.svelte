<script>
  import { onMount } from "svelte";

  let rfidData = [];
  let loading = true;
  let error = null;

  const dummyRfids = [
    { id: 1, rfid_number: "88697684", status: 1, created_at: "2025-10-10T13:00:00" },
    { id: 2, rfid_number: "09780647", status: 1, created_at: "2025-10-10T13:00:00" },
    { id: 3, rfid_number: "75834600", status: 0, created_at: "2025-10-10T13:00:00" },
    { id: 4, rfid_number: "90875490", status: "RFID NOT FOUND", created_at: "2025-10-10T13:00:00" },
  ];

  async function fetchRfids() {
    try {
      loading = true;
      await new Promise((r) => setTimeout(r, 600));
      rfidData = dummyRfids;
      loading = false;
    } catch (err) {
      error = "Failed to fetch RFID data.";
      loading = false;
    }
  }

  function toggleStatus(id) {
    rfidData = rfidData.map((item) =>
      item.id === id ? { ...item, status: item.status === 1 ? 0 : 1 } : item
    );
  }

  function formatDate(date) {
    return new Date(date).toLocaleString("en-US", {
      month: "short",
      day: "numeric",
      year: "numeric",
      hour: "2-digit",
      minute: "2-digit",
      hour12: true,
    });
  }

  onMount(() => fetchRfids());
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
      <h3>Active RFID</h3>
      {#if loading}
        <p>Loading...</p>
      {:else if error}
        <p class="error">{error}</p>
      {:else}
        {#each rfidData.slice(0, 3) as item, index}
          <div class="rfid-row">
            <span class="index">{index + 1}.</span>
            <span class="rfid-num">{item.rfid_number}</span>
            <label class="switch">
              <input
                type="checkbox"
                checked={item.status === 1}
                on:change={() => toggleStatus(item.id)}
              />
              <span class="slider"></span>
            </label>
          </div>
        {/each}
      {/if}
    </section>

    <!-- RIGHT PANEL -->
    <section class="right-panel">
      <h3>RFID Logs</h3>
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
            {#if loading}
              <tr><td colspan="4" class="center">Loading...</td></tr>
            {:else}
              {#each rfidData as item, i}
                <tr>
                  <td>{i + 1}</td>
                  <td>{item.rfid_number}</td>
                  <td>
                    {#if item.status === 1}
                      <span class="active">1</span>
                    {:else if item.status === 0}
                      <span class="inactive">0</span>
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
    </section>
  </div>
</main>

<style>
  :global(body) {
    margin: 0;
    font-family: "Poppins", sans-serif;
    background: #eef2f9;
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
  }

  .dashboard-header h1 {
    font-size: 1.9rem;
    font-weight: 600;
    color: #1e293b;
  }

  .codeblue {
    color: #2563eb;
    font-weight: 700;
    letter-spacing: 0.5px;
    font-size: 2.1rem;
  }

  /* CONTAINER */
  .container {
    display: flex;
    width: 90%;
    max-width: 1250px;
    background: #ffffff;
    border-radius: 16px;
    box-shadow: 0 8px 25px rgba(0, 0, 0, 0.08);
    overflow: hidden;
  }

  /* LEFT PANEL */
  .left-panel {
    flex: 0 0 300px;
    background: #f8fbff;
    padding: 24px;
    border-right: 2px solid #edf2f7;
  }

  .left-panel h3 {
    font-size: 1.1rem;
    margin-bottom: 16px;
    color: #2563eb;
    font-weight: 600;
  }

  .rfid-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    background: #ffffff;
    border-radius: 10px;
    box-shadow: 0 2px 6px rgba(0, 0, 0, 0.05);
    margin-bottom: 12px;
    padding: 10px 14px;
    transition: all 0.3s ease;
  }

  .rfid-row:hover {
    background: #eef7ff;
    transform: translateY(-2px);
  }

  .rfid-num {
    flex: 1;
    margin-left: 8px;
    font-weight: 500;
    font-size: 0.95rem;
  }

  .index {
    font-weight: 600;
    color: #2563eb;
  }

  /* SWITCH */
  .switch {
    position: relative;
    display: inline-block;
    width: 42px;
    height: 24px;
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
    background-color: #ccc;
    transition: 0.3s;
    border-radius: 24px;
  }

  .slider:before {
    position: absolute;
    content: "";
    height: 18px;
    width: 18px;
    left: 3px;
    bottom: 3px;
    background-color: white;
    transition: 0.3s;
    border-radius: 50%;
  }

  input:checked + .slider {
    background-color: #3b82f6;
  }

  input:checked + .slider:before {
    transform: translateX(18px);
  }

  /* RIGHT PANEL */
  .right-panel {
    flex: 1;
    padding: 24px;
  }

  .right-panel h3 {
    font-size: 1.1rem;
    margin-bottom: 16px;
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
    border-radius: 10px;
    overflow: hidden;
    table-layout: fixed;
  }

  th, td {
    padding: 12px 14px;
    font-size: 0.9rem;
    text-align: left;
    white-space: nowrap;
  }

  th {
    background: #2563eb;
    color: white;
    font-weight: 500;
  }

  td {
    border-bottom: 1px solid #e2e8f0;
  }

  tr:hover td {
    background: #f4f9ff;
  }

  .nowrap {
    white-space: nowrap;
  }

  .active {
    background: #22c55e;
    color: white;
    font-weight: 600;
    padding: 4px 10px;
    border-radius: 6px;
  }

  .inactive {
    background: #999;
    color: white;
    padding: 4px 10px;
    border-radius: 6px;
  }

  .notfound {
    background: #ef4444;
    color: white;
    padding: 4px 10px;
    border-radius: 6px;
    font-size: 0.85rem;
    white-space: nowrap;
  }

  .center {
    text-align: center;
    color: #888;
  }

  /* ✅ Mobile-Friendly Adjustments */
  @media (max-width: 900px) {
    .container {
      flex-direction: column;
      width: 95%;
    }

    .left-panel {
      width: 100%;
      border-right: none;
      border-bottom: 2px solid #edf2f7;
      text-align: center;
    }

    .rfid-row {
      justify-content: space-between;
      padding: 12px 18px;
    }

    .right-panel {
      width: 100%;
      padding: 16px;
    }

    table, th, td {
      font-size: 0.85rem;
    }

    th, td {
      padding: 10px 8px;
    }
  }

  @media (max-width: 600px) {
    .dashboard-header h1 {
      font-size: 1.4rem;
    }

    .codeblue {
      font-size: 1.6rem;
    }

    .container {
      width: 100%;
      border-radius: 0;
    }

    .rfid-row {
      flex-direction: row;
      justify-content: space-between;
    }
  }
</style>
