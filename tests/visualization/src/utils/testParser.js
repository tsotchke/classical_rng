export function parseTestResult(text) {
  const trimmed = text.trim();
  if (!trimmed) throw new Error('Paste one JSON result from a module test.');
  const result = JSON.parse(trimmed);
  if (!Array.isArray(result.buckets) || !Array.isArray(result.one_bits)) {
    throw new Error('Result must contain buckets and one_bits arrays.');
  }
  return result;
}
